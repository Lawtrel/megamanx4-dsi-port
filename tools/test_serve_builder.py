"""Loopback build smoke test using a synthetic BIN and a stand-in compiler."""
from hashlib import sha256
from http.client import HTTPConnection
from http import HTTPStatus
from pathlib import Path
import json
from tempfile import TemporaryDirectory
from threading import Thread
from unittest import TestCase
from unittest.mock import patch

import serve_builder as app


class BuilderTest(TestCase):
    def test_local_build_download_and_bad_dump(self):
        fixture = b"test dump"
        artifact = b"synthetic .nds"
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "tools").mkdir()
            (root / "tools/setup_native.py").write_text(
                "import sys\n"
                "from pathlib import Path\n"
                "assert Path(sys.argv[2]).read_bytes() == b'test dump'\n"
                "assert 'MODE2/2352' in Path(sys.argv[4]).read_text()\n",
                encoding="utf-8",
            )
            (root / "native").mkdir()
            (root / "native/compile.py").write_text(
                f"from pathlib import Path\noutput = Path({str(app.TARGET)!r})\n"
                f"output.parent.mkdir(parents=True)\noutput.write_bytes({artifact!r})\n",
                encoding="utf-8",
            )
            builder = app.Builder(root)
            with app.BuilderServer(("127.0.0.1", 0), builder) as server:
                authority = f"127.0.0.1:{server.server_port}"
                thread = Thread(target=server.serve_forever)
                thread.start()

                def request(path, body=None, **headers):
                    connection = HTTPConnection(authority, timeout=5)
                    try:
                        method = "GET" if body is None else "POST"
                        connection.request(method, path, body, headers)
                        response = connection.getresponse()
                        return response.status, response.getheader("Content-Disposition"), response.read()
                    finally:
                        connection.close()

                try:
                    with patch.multiple(app, AUTHORITY=authority, ORIGIN=f"http://{authority}",
                                        EXPECTED_SIZE=len(fixture),
                                        EXPECTED_SHA256=sha256(fixture).hexdigest()):
                        upload_headers = {"Origin": app.ORIGIN, "Content-Type": "application/octet-stream"}
                        self.assertEqual(request("/", **{"Sec-Fetch-Site": "cross-site"})[0], HTTPStatus.OK)
                        self.assertEqual(request("/status", Host="example.test")[0], HTTPStatus.FORBIDDEN)
                        self.assertEqual(request("/download")[0], HTTPStatus.NOT_FOUND)
                        self.assertEqual(request("/build", fixture, Origin="https://example.test")[0],
                                         HTTPStatus.FORBIDDEN)
                        self.assertEqual(request("/build", b"short", **upload_headers)[0], HTTPStatus.BAD_REQUEST)
                        self.assertEqual(request("/build", fixture, **upload_headers)[0], HTTPStatus.ACCEPTED)
                        builder.worker.join(5)
                        status, _, body = request("/status")
                        self.assertEqual(status, HTTPStatus.OK)
                        self.assertEqual(json.loads(body)["state"], "done")
                        self.assertEqual(request("/download"),
                                         (HTTPStatus.OK, f'attachment; filename="{app.TARGET.name}"', artifact))
                        self.assertEqual(request("/download", **{"Sec-Fetch-Site": "cross-site"})[0],
                                         HTTPStatus.FORBIDDEN)
                        self.assertEqual(request("/build", b"x" * len(fixture), **upload_headers)[0],
                                         HTTPStatus.UNPROCESSABLE_ENTITY)
                        self.assertEqual(request("/download")[0], HTTPStatus.NOT_FOUND)
                        self.assertFalse(builder.disc.exists())
                finally:
                    builder.stop.set()
                    server.shutdown()
                    thread.join()
                    if builder.worker:
                        builder.worker.join()

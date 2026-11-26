"""Find a sustained, sample-exact periodic tail in signed PCM16."""


def find_pcm_loop(pcm):
    """Return (start, period) in samples when the tail has three full cycles."""
    values = memoryview(pcm).cast('h')[::-1]
    size = len(values)
    if size < 24:
        return None

    # Exclude flat tails, including fade-outs ending in silence.
    constant = 1
    while constant < size and values[constant] == values[0]:
        constant += 1
    first = max(8, constant + 1)
    if first > size // 3:
        return None

    # A Z-function on reversed PCM finds suffix matches in linear time.
    matches = [0] * (size // 3 + 1)
    left = right = 0
    for period in range(1, len(matches)):
        length = min(right - period, matches[period - left]) if period < right else 0
        while period + length < size and values[length] == values[period + length]:
            length += 1
        matches[period] = length
        if period + length > right:
            left, right = period, period + length
        if period >= first and length >= 2 * period:
            return size - period - length, period
    return None

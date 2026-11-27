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


def find_reverb_loop(pcm, dry_period):
    """Find three wet cycles within one PCM16 unit of the final cycle."""
    if not isinstance(dry_period, int) or dry_period < 8:
        return None
    samples = memoryview(pcm).cast('h')
    best = None
    for period in (dry_period, 2 * dry_period):
        if len(samples) < 3 * period:
            continue
        tail = len(samples) - period
        cycle = samples[tail:]
        if max(cycle) - min(cycle) <= 2:
            continue
        start = tail
        while start:
            previous = start - 1
            if abs(samples[previous] - cycle[(previous - tail) % period]) > 1:
                break
            start = previous
        if len(samples) - start >= 3 * period and (best is None or start + period < sum(best)):
            best = start, period
    return best

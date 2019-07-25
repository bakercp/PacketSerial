# TODO

First check out the [Help Wanted](../../../issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22) tag in the issues section for specific ideas.

## Future Plans

Right now the encoder / decoder works by encoding or decoding a full buffer. SLIP (and COBS?) encoding, for instance, can be encoded / decoded on the fly, allowing for a smaller buffer allocation. https://github.com/CNMAT/OSC uses an "on-the-fly" approach, rather than a large buffer approach. It would be interesting to investigate this.

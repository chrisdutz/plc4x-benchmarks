
# Benchmarks

## Java

### PLC4X

### Moka7

Only seems to support single-item reads.
No real built-in support for more complex types such as Float, Double, temporal values etc.

### S7Connector

Only seems to support single-item reads.
No real built-in support for more complex types such as Float, Double, temporal values etc.

## Cpp

### Snap7

Seems to support multi-item reads.
Hard limit of 20 tags per multi-item-read.
It seems Snap7 doesn't support the 64-bit types such as LWORD, LINT, ULINT, LREAL, ...
Also does it not support the 16-bit character types WCHAR and WSTRING
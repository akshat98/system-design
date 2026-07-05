# Local Key-Value Latency Report

## Summary

This report captures a local benchmark comparing Redis, SQLite, and PostgreSQL for simple key/value retrieval on the current machine.

## Environment

- OS: macOS
- Python: 3.14.2
- Redis: 8.8.0 (local service)
- SQLite: 3.51.0
- PostgreSQL: 14.20 (local service)

## Benchmark details

- Samples per backend: 1000
- Value size: about 256 bytes
- Measurement: end-to-end read latency for retrieving a stored value by key

## Verified results

| Backend | Avg (ms) | Median (ms) | P95 (ms) | Min (ms) | Max (ms) |
| --- | ---: | ---: | ---: | ---: | ---: |
| Redis | 0.0327 | 0.0317 | 0.0386 | 0.0235 | 0.2166 |
| SQLite | 0.0044 | 0.0038 | 0.0085 | 0.0033 | 0.0580 |
| PostgreSQL | 0.0316 | 0.0286 | 0.0333 | 0.0230 | 2.4204 |

## Notes

- Redis results reflect a local TCP loopback access path.
- SQLite results reflect local file-backed database access.
- PostgreSQL results reflect a local TCP connection to a running server.
- For this simple single-key read benchmark, SQLite was fastest on this machine.
- Redis and PostgreSQL were close on median latency, but PostgreSQL showed higher tail latency in this local test.

## Re-run

```bash
cd /Users/apple/Documents/Projects/system-design/cache
source .venv/bin/activate
python benchmark_kv.py
```

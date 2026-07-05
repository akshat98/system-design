# Local key-value latency benchmark

This folder contains a simple local benchmark for comparing Redis, SQLite, and PostgreSQL for key/value lookups.

## Prerequisites

- Python 3
- Redis running locally
- PostgreSQL running locally
- SQLite is built in

## Start Redis locally

```bash
cd cache
docker compose up -d
```

If Docker is not available, Redis can also be started with Homebrew:

```bash
brew services start redis
```

## Start PostgreSQL locally

```bash
brew services start postgresql@14
```

## Run the benchmark

```bash
source .venv/bin/activate
python benchmark_kv.py
```

The script writes a local SQLite database at `cache/kv.sqlite3` and measures read latency for 1000 key/value lookups against Redis, SQLite, and PostgreSQL.

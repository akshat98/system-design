import statistics
import sqlite3
import time
from pathlib import Path

import psycopg2
import redis

SAMPLE_SIZE = 1000
VALUE_SIZE = 256
REDIS_HOST = "127.0.0.1"
REDIS_PORT = 6379
SQLITE_DB = Path(__file__).with_name("kv.sqlite3")
POSTGRES_CONN = {
    "host": "127.0.0.1",
    "port": 5432,
    "database": "postgres",
    "user": "apple",
}


def build_payload() -> str:
    return "x" * VALUE_SIZE


def benchmark_redis() -> dict:
    client = redis.Redis(host=REDIS_HOST, port=REDIS_PORT, decode_responses=True)
    client.flushdb()

    for index in range(SAMPLE_SIZE):
        client.set(f"key:{index}", build_payload())

    latencies_ms = []
    for index in range(SAMPLE_SIZE):
        start = time.perf_counter_ns()
        client.get(f"key:{index}")
        elapsed_ms = (time.perf_counter_ns() - start) / 1_000_000
        latencies_ms.append(elapsed_ms)

    return summarize(latencies_ms)


def benchmark_sqlite() -> dict:
    if SQLITE_DB.exists():
        SQLITE_DB.unlink()

    conn = sqlite3.connect(SQLITE_DB)
    conn.execute("DROP TABLE IF EXISTS kv")
    conn.execute("CREATE TABLE kv (key TEXT PRIMARY KEY, value TEXT NOT NULL)")

    rows = [(f"key:{index}", build_payload()) for index in range(SAMPLE_SIZE)]
    conn.executemany("INSERT INTO kv(key, value) VALUES (?, ?)", rows)
    conn.commit()

    latencies_ms = []
    for index in range(SAMPLE_SIZE):
        start = time.perf_counter_ns()
        conn.execute("SELECT value FROM kv WHERE key = ?", (f"key:{index}",)).fetchone()
        elapsed_ms = (time.perf_counter_ns() - start) / 1_000_000
        latencies_ms.append(elapsed_ms)

    conn.close()
    return summarize(latencies_ms)


def benchmark_postgres() -> dict:
    conn = psycopg2.connect(**POSTGRES_CONN)
    conn.autocommit = True
    with conn.cursor() as cursor:
        cursor.execute("DROP TABLE IF EXISTS kv")
        cursor.execute("CREATE TABLE kv (key TEXT PRIMARY KEY, value TEXT NOT NULL)")
        cursor.executemany(
            "INSERT INTO kv(key, value) VALUES (%s, %s)",
            [(f"key:{index}", build_payload()) for index in range(SAMPLE_SIZE)],
        )

    latencies_ms = []
    with conn.cursor() as cursor:
        for index in range(SAMPLE_SIZE):
            start = time.perf_counter_ns()
            cursor.execute("SELECT value FROM kv WHERE key = %s", (f"key:{index}",))
            cursor.fetchone()
            elapsed_ms = (time.perf_counter_ns() - start) / 1_000_000
            latencies_ms.append(elapsed_ms)

    conn.close()
    return summarize(latencies_ms)


def summarize(latencies_ms: list[float]) -> dict:
    return {
        "samples": len(latencies_ms),
        "min_ms": round(min(latencies_ms), 4),
        "max_ms": round(max(latencies_ms), 4),
        "avg_ms": round(statistics.mean(latencies_ms), 4),
        "median_ms": round(statistics.median(latencies_ms), 4),
        "p95_ms": round(statistics.quantiles(latencies_ms, n=100)[94], 4),
    }


def main() -> None:
    print("Measuring key/value retrieval latency")
    print(f"- samples per backend: {SAMPLE_SIZE}")
    print("- value size: ~{} bytes".format(VALUE_SIZE))
    print()

    redis_summary = benchmark_redis()
    print("Redis (local TCP loopback)")
    print(f"  avg_ms: {redis_summary['avg_ms']}")
    print(f"  median_ms: {redis_summary['median_ms']}")
    print(f"  p95_ms: {redis_summary['p95_ms']}")
    print(f"  min_ms: {redis_summary['min_ms']}")
    print(f"  max_ms: {redis_summary['max_ms']}")
    print()

    sqlite_summary = benchmark_sqlite()
    print("SQLite (local file-backed)")
    print(f"  avg_ms: {sqlite_summary['avg_ms']}")
    print(f"  median_ms: {sqlite_summary['median_ms']}")
    print(f"  p95_ms: {sqlite_summary['p95_ms']}")
    print(f"  min_ms: {sqlite_summary['min_ms']}")
    print(f"  max_ms: {sqlite_summary['max_ms']}")
    print()

    postgres_summary = benchmark_postgres()
    print("PostgreSQL (local TCP)")
    print(f"  avg_ms: {postgres_summary['avg_ms']}")
    print(f"  median_ms: {postgres_summary['median_ms']}")
    print(f"  p95_ms: {postgres_summary['p95_ms']}")
    print(f"  min_ms: {postgres_summary['min_ms']}")
    print(f"  max_ms: {postgres_summary['max_ms']}")


if __name__ == "__main__":
    main()

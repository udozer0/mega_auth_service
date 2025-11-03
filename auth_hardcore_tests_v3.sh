#!/usr/bin/env bash
set -euo pipefail

HOST="${1:-127.0.0.1}"
PORT="${2:-8082}"
[[ "$HOST" == "0.0.0.0" ]] && HOST="127.0.0.1"
BASE="http://$HOST:$PORT"

NC_BIN="${NC_BIN:-nc}"
AB_BIN="${AB_BIN:-ab}"

# Разбираем опции nc (OpenBSD/GNU)
HELP="$($NC_BIN -h 2>&1 || true)"
NC_OPTS="-w 5"
if echo "$HELP" | grep -q -- "-q "; then
  NC_OPTS="$NC_OPTS -q 2"      # подождать 2s после EOF stdin
fi
if echo "$HELP" | grep -q -- "-C"; then
  NC_OPTS="$NC_OPTS -C"        # CRLF как в HTTP
fi

echo "===== A) Read-timeout ====="
# Вариант 1: Python — самый честный (предпочтительно)
if command -v python3 >/dev/null 2>&1; then
  python3 - "$HOST" "$PORT" <<'PY'
import socket, sys, time
h, p = sys.argv[1], int(sys.argv[2])
s = socket.create_connection((h, p))
s.sendall(b"GET /health HTTP/1.1\r\n")  # без \r\n\r\n
t = time.time(); s.settimeout(5.0)
try:
    d = s.recv(4096)
    print((d.decode(errors="ignore").splitlines() or ["<no data>"])[0])
except Exception as e:
    print("closed:", e)
print("elapsed:", round(time.time()-t,3), "s")
s.close()
PY
else
  # Вариант 2: держим stdin открытым, чтобы nc дождался ответа сервера
  { printf 'GET /health HTTP/1.1\r\n'; sleep 1; } | $NC_BIN $NC_OPTS "$HOST" "$PORT" | head -c 256
fi

echo "===== B) Idle keep-alive (>3s) ====="
COUNT=$({ printf "GET /health HTTP/1.1\r\nHost: x\r\n\r\n"; sleep 4; printf "GET /health HTTP/1.1\r\nHost: x\r\n\r\n"; } \
| $NC_BIN $NC_OPTS "$HOST" "$PORT" | grep -a -c "HTTP/1" || true)
echo "$COUNT"
echo "===== C) ab -k ====="
AB_PATH=$(command -v "$AB_BIN" || true)
if [[ -z "$AB_PATH" ]]; then
  echo "SKIP: ab not found. Install: apt-get install -y apache2-utils"
else
  echo "Using ab: $AB_PATH"
  set +e  # не падаем, даже если ab вернёт non-zero
  "$AB_BIN" -k -n 60 -c 1 "$BASE/health"
  AB_RC=$?
  set -e
  echo "(ab exit code: $AB_RC)"
fi

WRK_BIN="${WRK_BIN:-wrk}"
WRK_PATH=$(command -v "$WRK_BIN" || true)
echo "===== D) keep-alive benchmark ====="
if [[ -n "$AB_PATH" ]]; then
  echo "Using ab: $AB_PATH"
  "$AB_BIN" -k -n 60 -c 1 "$BASE/health"
elif [[ -n "$WRK_PATH" ]]; then
  echo "Using wrk: $WRK_PATH"
  "$WRK_BIN" -t2 -c32 -d10s --timeout 2s --latency "$BASE/health"
else
  echo "SKIP: no ab or wrk found."
fi

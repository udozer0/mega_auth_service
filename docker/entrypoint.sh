#!/usr/bin/env bash
set -euo pipefail

# PM_CONNSTR должен быть вроде: host=db port=5432 dbname=pm user=pm password=pm
if [ -z "${PM_CONNSTR:-}" ]; then
  echo "PM_CONNSTR is not set" >&2
  exit 1
fi

# Подождать БД
echo "Waiting for Postgres..."
for i in $(seq 1 60); do
  if psql "$PM_CONNSTR" -c "select 1" >/dev/null 2>&1; then
    break
  fi
  sleep 1
done

# Накатить схему (идемпотентно)
if [ -f /opt/mega-auth/config/000_init.sql ]; then
  echo "Applying migrations..."
  psql "$PM_CONNSTR" -v ON_ERROR_STOP=1 -f /opt/mega-auth/config/000_init.sql || {
    echo "Migrations failed"; exit 1;
  }
fi

# Старт сервиса
echo "Starting mega_auth..."
exec /opt/mega-auth/mega_auth

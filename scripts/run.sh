#!/usr/bin/env bash
set -euo pipefail
export PM_CONNSTR="${PM_CONNSTR:-host=localhost port=5432 dbname=pm user=pm password=pm}"
./pm_auth

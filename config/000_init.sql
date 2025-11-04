CREATE EXTENSION IF NOT EXISTS citext;
CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE IF NOT EXISTS pm_users (
    id              UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    email           CITEXT UNIQUE NOT NULL,
    password_hash   TEXT NOT NULL,
    password_algo   TEXT NOT NULL,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    email_verified  BOOLEAN NOT NULL DEFAULT false
);

CREATE TABLE IF NOT EXISTS pm_email_verifications (
    user_id     UUID REFERENCES pm_users(id) ON DELETE CASCADE,
    token       TEXT NOT NULL,
    expires_at  TIMESTAMPTZ NOT NULL,
    used        BOOLEAN NOT NULL DEFAULT false,
    PRIMARY KEY (user_id, token)
);

CREATE TABLE IF NOT EXISTS user_certs (
    id            UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id       UUID NOT NULL REFERENCES pm_users(id),
    serial        TEXT NOT NULL,
    subject_dn    TEXT,
    public_key    TEXT,
    issued_at     TIMESTAMPTZ NOT NULL DEFAULT now(),
    expires_at    TIMESTAMPTZ NOT NULL,
    revoked       BOOLEAN NOT NULL DEFAULT false,
    revoke_reason TEXT,
    revoked_at    TIMESTAMPTZ,
    meta_json     JSONB
);
CREATE INDEX IF NOT EXISTS idx_user_certs_user ON user_certs(user_id);
CREATE INDEX IF NOT EXISTS idx_user_certs_serial ON user_certs(serial);

CREATE TABLE IF NOT EXISTS user_webauthn (
    id             UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id        UUID NOT NULL REFERENCES pm_users(id),
    credential_id  TEXT NOT NULL UNIQUE,
    public_key_jwk TEXT NOT NULL,
    created_at     TIMESTAMPTZ NOT NULL DEFAULT now()
);

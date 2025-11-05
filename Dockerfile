# ---------- build stage ----------
FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake pkg-config git curl ca-certificates \
    libboost-system-dev libpqxx-dev libsodium-dev nlohmann-json3-dev && \
    rm -rf /var/lib/apt/lists/*


WORKDIR /app
# Копируем только то, что нужно для сборки
COPY CMakeLists.txt /app/
COPY src /app/src
COPY include /app/include
COPY third_party /app/third_party
COPY config /app/config
COPY openapi /app/openapi

# Чистая out-of-source сборка, без старого кэша
RUN rm -rf /app/build && \
    cmake -S /app -B /app/build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build /app/build -j


# ---------- runtime stage ----------
FROM ubuntu:24.04 AS runtime
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    libboost-system-dev libpqxx-dev libsodium23 \
    postgresql-client curl ca-certificates && \
    rm -rf /var/lib/apt/lists/*

RUN useradd -m appuser
WORKDIR /opt/mega-auth

COPY --from=build /app/build/mega_auth /opt/mega-auth/mega_auth
COPY --from=build /app/config /opt/mega-auth/config
COPY docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh && chown -R appuser:appuser /opt/mega-auth
USER appuser

EXPOSE 8080
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

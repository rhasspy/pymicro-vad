FROM quay.io/pypa/manylinux_2_28_x86_64 as build-amd64

ENV LANG C.UTF-8
ENV MANYLINUX_PLATFORM manylinux_2_28_x86_64

FROM quay.io/pypa/manylinux_2_28_aarch64 as build-arm64

ENV LANG C.UTF-8
ENV MANYLINUX_PLATFORM manylinux_2_28_aarch64

# -----------------------------------------------------------------------------
# Python 3.9
# -----------------------------------------------------------------------------

ARG TARGETARCH
ARG TARGETVARIANT
FROM build-${TARGETARCH}${TARGETVARIANT} as python39

# Build
WORKDIR /build
COPY ./ ./
RUN python3.9 -m venv venv && \
    venv/bin/pip3 install --upgrade pip && \
    venv/bin/pip3 install --upgrade build wheel auditwheel && \
    venv/bin/python3 -m build --wheel
RUN find dist -name '*.whl' | xargs auditwheel repair --plat "${MANYLINUX_PLATFORM}"

# Test
WORKDIR /test
COPY ./tests/ ./tests/
RUN python3.9 -m venv venv && \
    venv/bin/pip3 install --upgrade pip && \
    venv/bin/pip3 install --upgrade wheel pytest && \
    venv/bin/pip3 install --no-index pymicro-vad -f /build/wheelhouse/ && \
    venv/bin/pytest tests

# -----------------------------------------------------------------------------
# Python 3.10
# -----------------------------------------------------------------------------

ARG TARGETARCH
ARG TARGETVARIANT
FROM build-${TARGETARCH}${TARGETVARIANT} as python310

# Build
WORKDIR /build
COPY ./ ./
RUN python3.10 -m venv venv && \
    venv/bin/pip3 install --upgrade pip && \
    venv/bin/pip3 install --upgrade build wheel auditwheel && \
    venv/bin/python3 -m build --wheel
RUN find dist -name '*.whl' | xargs auditwheel repair --plat "${MANYLINUX_PLATFORM}"

# Test
WORKDIR /test
COPY ./tests/ ./tests/
RUN python3.10 -m venv venv && \
    venv/bin/pip3 install --upgrade pip && \
    venv/bin/pip3 install --upgrade wheel pytest && \
    venv/bin/pip3 install --no-index pymicro-vad -f /build/wheelhouse/ && \
    venv/bin/pytest tests

# -----------------------------------------------------------------------------
# Python 3.11
# -----------------------------------------------------------------------------

ARG TARGETARCH
ARG TARGETVARIANT
FROM build-${TARGETARCH}${TARGETVARIANT} as python311

# Build
WORKDIR /build
COPY ./ ./
RUN python3.11 -m venv venv && \
    venv/bin/pip3 install --upgrade pip && \
    venv/bin/pip3 install --upgrade build wheel auditwheel && \
    venv/bin/python3 -m build --wheel
RUN find dist -name '*.whl' | xargs auditwheel repair --plat "${MANYLINUX_PLATFORM}"

# Test
WORKDIR /test
COPY ./tests/ ./tests/
RUN python3.11 -m venv venv && \
    venv/bin/pip3 install --upgrade pip && \
    venv/bin/pip3 install --upgrade wheel pytest && \
    venv/bin/pip3 install --no-index pymicro-vad -f /build/wheelhouse/ && \
    venv/bin/pytest tests

# -----------------------------------------------------------------------------
# Python 3.12
# -----------------------------------------------------------------------------

ARG TARGETARCH
ARG TARGETVARIANT
FROM build-${TARGETARCH}${TARGETVARIANT} as python312

# Build
WORKDIR /build
COPY ./ ./
RUN python3.12 -m venv venv && \
    venv/bin/pip3 install --upgrade pip && \
    venv/bin/pip3 install --upgrade build wheel auditwheel && \
    venv/bin/python3 -m build --wheel
RUN find dist -name '*.whl' | xargs auditwheel repair --plat "${MANYLINUX_PLATFORM}"

# Test
WORKDIR /test
COPY ./tests/ ./tests/
RUN python3.12 -m venv venv && \
    venv/bin/pip3 install --upgrade pip && \
    venv/bin/pip3 install --upgrade wheel pytest && \
    venv/bin/pip3 install --no-index pymicro-vad -f /build/wheelhouse/ && \
    venv/bin/pytest tests

# -----------------------------------------------------------------------------

FROM scratch
ARG TARGETARCH
ARG TARGETVARIANT

COPY --from=python39 /build/wheelhouse/ ./
COPY --from=python310 /build/wheelhouse/ ./
COPY --from=python311 /build/wheelhouse/ ./
COPY --from=python312 /build/wheelhouse/ ./

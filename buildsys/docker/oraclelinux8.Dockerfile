# Oracle Linux 8 toolchain with a vcpkg binary cache.
# Qt is compiled against glibc 2.28 for RHEL 8 compatibility.

FROM oraclelinux:8

ENV US3_TOOLCHAIN_ROOT=/opt/us3-toolchain \
    US3_VCPKG_CACHE=/opt/us3-toolchain/vcpkg-cache \
    US3_VCPKG_ROOT=/opt/us3-toolchain/vcpkg \
    US3_VCPKG_DOWNLOADS=/opt/us3-toolchain/downloads \
    CI=true

# Install the prerequisites for scripts/bootstrap-linux.sh.
RUN dnf -y install git python3.9 sudo which findutils tar zstd \
    && dnf clean all \
    && alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 100

WORKDIR /src

# Key this layer to dependency inputs.
COPY vcpkg.json vcpkg-configuration.json ./
COPY buildsys/toolchain.lock.json        ./buildsys/
COPY buildsys/vcpkg/overlay-ports        ./buildsys/vcpkg/overlay-ports
COPY qwtplot3d                           ./qwtplot3d
COPY admin/cmake/triplets                ./admin/cmake/triplets
COPY scripts/bootstrap-linux.sh          ./scripts/
COPY scripts/build-toolchain.sh          ./scripts/

RUN chmod +x scripts/*.sh && bash scripts/bootstrap-linux.sh

# Compile the dependency graph for both profiles this platform ships.
# Remove build scratch to reduce the final image size.
RUN bash scripts/build-toolchain.sh \
        --cache-dir "${US3_VCPKG_CACHE}" \
        --qt6 \
        --profiles "APP HPC" \
        --skip-bootstrap \
 && rm -rf "${US3_VCPKG_ROOT}/buildtrees" \
           "${US3_VCPKG_ROOT}/packages" \
           "${US3_VCPKG_ROOT}"/installed-toolchain-* \
           "${US3_VCPKG_DOWNLOADS}" \
 && du -sh "${US3_VCPKG_CACHE}"

WORKDIR /
RUN rm -rf /src

# Validate the cache while building the image.
RUN test -d "${US3_VCPKG_CACHE}" \
 && test "$(find "${US3_VCPKG_CACHE}" -name '*.zip' | wc -l)" -gt 0 \
 && echo "toolchain cache OK: $(find "${US3_VCPKG_CACHE}" -name '*.zip' | wc -l) packages"

# Link the GHCR package to the publishing repository.
ARG IMAGE_SOURCE=https://github.com/ehb54/ultrascan3

LABEL org.opencontainers.image.title="UltraScan3 build toolchain (Oracle Linux 8)" \
      org.opencontainers.image.description="Prebuilt Qt6 and C dependencies for UltraScan3 RHEL8 builds" \
      org.opencontainers.image.source="${IMAGE_SOURCE}"

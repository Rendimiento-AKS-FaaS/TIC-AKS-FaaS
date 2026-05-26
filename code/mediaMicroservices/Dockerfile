FROM yg397/thrift-microservice-deps:xenial

COPY ./ /media-microservices
WORKDIR /media-microservices

RUN rm -rf gen-cpp gen-lua \
    && mkdir -p gen-cpp gen-lua \
    && thrift -r --gen cpp -out gen-cpp media_service.thrift \
    && thrift -r --gen lua -out gen-lua media_service.thrift \
    && find gen-lua -maxdepth 1 -type f \
         ! -name 'media_service_PageService.lua' \
         ! -name 'media_service_ttypes.lua' \
         ! -name 'media_service_constants.lua' -delete \
    && mkdir -p build \
    && cd build \
    && cmake .. \
    && make -j"$(nproc)" \
    && make install

WORKDIR /media-microservices

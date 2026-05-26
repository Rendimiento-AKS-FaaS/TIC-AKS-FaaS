FROM yg397/openresty-thrift:xenial

# Copiar archivos generados de lua (requiere contexto de build en la raíz del proyecto)
COPY ./gen-lua /gen-lua

# Copiar los scripts y config de Nginx
COPY ./nginx-web-server/lua-scripts /usr/local/openresty/nginx/lua-scripts
COPY ./nginx-web-server/conf/nginx.conf /usr/local/openresty/nginx/conf/nginx.conf
COPY ./nginx-web-server/jaeger-config.json /usr/local/openresty/nginx/jaeger-config.json

EXPOSE 8080

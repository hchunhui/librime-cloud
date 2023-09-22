FROM ubuntu:16.04
RUN apt-get update && apt-get install -qq build-essential mingw-w64 zip --no-install-recommends

FROM debian:bullseye AS build

RUN apt-get update && \
	apt-get install cmake g++ libboost-dev libboost-program-options-dev --yes

RUN mkdir /home/build
WORKDIR /home/build

COPY ./CMakeLists.txt /home/
COPY ./src /home/src

RUN cmake .. && make revelation-server -j4

FROM debian:bullseye AS server

COPY --from=build /home/build/src/revelation-server /home/revelation-server
COPY ./viewer /home/viewer

CMD [ "/home/revelation-server", "0.0.0.0" ,"8000", "/home/viewer" ]
EXPOSE 8000

FROM debian:bullseye AS build

RUN apt-get update && \
	apt-get install cmake g++ libboost-dev --yes

RUN mkdir /home/build
WORKDIR /home/build

COPY ./CMakeLists.txt /home/
COPY ./src /home/src

RUN cmake .. && make revelation -j4

FROM debian:bullseye AS server

COPY --from=build /home/build/src/revelation /home/revelation

CMD [ "/home/revelation" ]
EXPOSE 8000
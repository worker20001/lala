FROM python:3.11-slim

RUN apt-get update && apt-get install -y g++ make

WORKDIR /app

COPY . .

RUN pip install -r requirements.txt
RUN g++ -shared -fPIC -std=c++14 -pthread -o soul.so soul.cpp

CMD waitress-serve --port=$PORT api:app

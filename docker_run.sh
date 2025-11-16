docker run -it \
  -p 7777:7777/udp \
  --name docker-server-container \
  -v $(pwd):/project \
  docker-server

#docker start -ai docker-server-container
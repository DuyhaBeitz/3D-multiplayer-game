docker run -it \
  -p 7777:7777/udp \
  --name docker-cross-container \
  -v $(pwd):/project \
  docker-cross

#docker start -ai docker-cross-container
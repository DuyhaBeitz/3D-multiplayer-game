# Build the image
#sudo systemctl start docker

sudo docker build --no-cache -t docker-server .
sudo docker run -it -p 7777:7777/udp --name docker-server docker-server

#sudo docker run -it --name docker-server --network host docker-server
#sudo sudo docker run -it --name docker-server -p 7777:7777 docker-server


#sudo docker build -t docker-server .

# Stop and remove the old container if it exists
#sudo docker stop game-server-name 2>/dev/null || true
#sudo docker rm game-server-name 2>/dev/null || true

# Run the new container
#sudo docker run -it --name game-server-name docker-server
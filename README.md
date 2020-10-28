# Ermiry's Tiny Things API Service

### Development
```
sudo docker run \
  -it \
  --name things --rm \
  -p 5002 --net ermiry \
  -v /home/ermiry/Documents/ermiry/website/ermiry-website/tiny-things-api:/home/things \
  -v /home/ermiry/Documents/ermiry/website/ermiry-website/jwt:/home/things/keys \
  -e CURR_ENV=development \
  -e PORT=5002 \
  -e PRIV_KEY=/home/things/keys/key.key -e PUB_KEY=/home/things/keys/key.pub \
  -e MONGO_URI=mongodb://things:thingspassword@192.168.100.39:27017/ermiry \
  -e CERVER_RECEIVE_BUFFER_SIZE=4096 -e CERVER_TH_THREADS=4 \
  ermiry/tiny-things-api:development /bin/bash
```
# Ermiry's Tiny Things API Service

### Development
```
sudo docker run \
  -it \
  --name things --rm \
  -p 5000:5000 \
  -v /home/ermiry/Documents/ermiry/website/tiny-things-api:/home/things \
  -e CURR_ENV=development \
  -e PORT=5000 \
  -e PRIV_KEY=/home/things/keys/key.key -e PUB_KEY=/home/things/keys/key.pub \
  -e MONGO_APP_NAME=api -e MONGO_DB=things \
  -e MONGO_URI=mongodb://api:password@192.168.100.39:27017/things \
  -e CERVER_RECEIVE_BUFFER_SIZE=4096 -e CERVER_TH_THREADS=4 \
  ermiry/tiny-things-api:development /bin/bash
```

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

## Routes

### Main

#### GET /api/pocket
**Access:** Public \
**Description:** Pocket top level route \
**Returns:**
  - 200 on success

#### GET api/pocket/version
**Access:** Public \
**Description:** Returns pocket-api service current version \
**Returns:**
  - 200 and version's json on success

#### GET api/pocket/auth
**Access:** Private \
**Description:** Used to test if jwt keys work correctly \
**Returns:**
  - 200 on success
  - 401 on failed auth
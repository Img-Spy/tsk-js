FROM node:6.11.3

ENV NPM_CONFIG_PREFIX=/home/node/.npm-global
ENV PATH=${PATH}:/home/node/.npm-global/bin

RUN apt-get update && apt-get install nano

USER node
RUN mkdir ~/.npm-global \
    && mkdir ~/app \
    && npm install -g typescript ts-node forever nodemon mocha

RUN npm install -g fsbash-cli

WORKDIR /home/node/app/
RUN ln -s /data \
    && npm init -y \
    && npm install tsk-js

USER root

CMD ["node", "-e", "const { TSK } = require(\"tsk-js\")", "-i"]

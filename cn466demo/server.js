'use strict';

const express = require('express');
const mqtt = require('mqtt');
require('dotenv').config();
const { MongoClient } = require("mongodb");
const line = require('@line/bot-sdk');
const ngrok = require('ngrok');



// Constants
const PORT = 8080;
const HOST = '0.0.0.0';
const line_cfg = {
    channelAccessToken: process.env.LINE_ACCESS_TOKEN,
    channelSecret: process.env.LINE_CH_SECRET
};

// App
var data_idx = 0;
var temp_value = 0;
var humid_value = 0;
const app = express();

app.use('/liff',express.static('liff'));

app.get('/api/sensor',async(req, res) => {
    const ans = {idx:data_idx , temp:temp_value , humid:humid_value}
    res.send(JSON.stringify(ans))
})

app.get('/', async (req, res) => {

    const value = parseInt(req.query.value)
    const results = await try_query(value)
    const summary = results.map(a => a.timestamp)
    console.log(results)
    res.send(summary.toString());

});



//MQTT
const mqttclient  = mqtt.connect('mqtt://cn466demo-mosquitto-1');
mqttclient.on('connect', function () {
    mqttclient.subscribe('cn466/sensor', function (err) {
        if (!err) {
            const obj = {status:'ok'};
            mqttclient.publish('cn466/status', JSON.stringify(obj))
        }
    })
})
mqttclient.on('message', function (topic, message) {
    // message is Buffer
    console.log(message.toString());
    const msg = JSON.parse(message.toString());
    try_insert(msg.temp).catch(console.dir);
    data_idx = data_idx + 1;
    temp_value = msg.temp;
    humid_value = msg.humid;
    lineClient.pushMessage(process.env.USER_ID,{type:'text',text:msg.temp.toString()})
})


//database
const mongodbclient = new MongoClient(process.env.MONGODB_URI);
async function try_connect() {
    try {
        await mongodbclient.connect();
        const database = mongodbclient.db('cn466');
        try{
            await database.createCollection("sensor")    
            console.log("Collection created!");  
            
        } catch (err) {
            console.log("Collection existed")
        }
    }
    finally{
        mongodbclient.close();
    }
}

try_connect().catch(console.dir);

async function try_X(value) {
    try {
        await mongodbclient.connect();
        const database = mongodbclient.db('cn466');
        const sensor = database.collection('sensor');
        //do x
    }
    finally{
        await mongodbclient.close();
    }
}

async function try_insert(value) {
    try {
        await mongodbclient.connect();
        const database = mongodbclient.db('cn466');
        const sensor = database.collection('sensor');
        const doc = {   
                        timestamp: new Date(),
                        value : value
                    }
        console.log(doc);
        const result = await sensor.insertOne(doc);
        console.log(result);
    }
    finally{
        await mongodbclient.close();
    }
}

async function try_query(value) {

    var results = []

    try {

        await mongodbclient.connect();
        const database = mongodbclient.db('cn466');
        const sensor = database.collection('sensor')
        const cond = {value: value}
        results = await sensor.find(cond).toArray()    

    } finally {
        await mongodbclient.close()
    }

    //console.log(results)  
    return results

}

//line
const lineClient = new line.Client(line_cfg);
app.post('/callback', line.middleware(line_cfg), (req, res) => {
    Promise
      .all(req.body.events.map(handleEvent))
      .then((result) => res.json(result))
      .catch((err) => {
        console.error(err);
        res.status(500).end();
      });
});

function handleEvent(event) {
    if (event.type !== 'message' || event.message.type !== 'text') {
      // ignore non-text-message event
      return Promise.resolve(null);
    }
  
    // create a echoing text message
    const echo = { type: 'text', text: event.message.text };
  
    // use reply API
    return lineClient.replyMessage(event.replyToken, echo);
}

app.listen(PORT, () => {
    console.log("It seems that BASE_URL is not set. Connecting to ngrok...")
    ngrok.connect({addr:PORT, authtoken:process.env.NGROK_AUTH_TOKEN}).then(url => {
    console.log('listening on ' + url + '/callback');
    lineClient.setWebhookEndpointUrl(url + '/callback');
    }).catch(console.error);
});


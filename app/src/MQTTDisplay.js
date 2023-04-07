import React, { useState, useEffect } from 'react';
import Amplify, { Auth } from 'aws-amplify';
import awsmobile from './aws-exports';
import AWSIoTData from 'aws-iot-device-sdk';
import AWSConfiguration from './aws-iot-configuration.js';
import { List, ListItem, Card, Toggle, BlockTitle } from 'konsta/react';

Amplify.configure(awsmobile);

/*
    Note - I attempted to use Amplify PubSub for IoT message handling but found that
    it lacked adequate functionality to handle multiple subscriptions easily. Therefore, 
    I opted to use aws-iot-devide-sdk which proved much easier to use. 
*/

//######################################################################################
function arrayRemove(arr, value) {
  // REMOVE SPECIFIC ITEM BY VALUE FROM AN ARRAY
  //https://love2dev.com/blog/javascript-remove-from-array/
  return arr.filter(function(ele){
      return ele !== value;
  });

}
//######################################################################################

function MQTTDisplay(props) {
  // ALLOW USER TO SUBSCRIBE TO MQTT TOPICS

  const [desiredSubscriptionTopic, setDesiredSubscriptionTopic] = useState("#");
  const [desiredPublishTopic,   setDesiredPublishTopic]   = useState("Door");
  const [desiredPublishMessage, setDesiredPublishMessage] = useState(`{ "status": 1 }`);
  const [subscribedTopics, setSubscribedTopics]           = useState([]);
  
  // isConnected and mqttClient strictly used for publishing;
  // Subscriptions are instead handled in child MQTTSubscription components
  const [isConnected, setIsConnected]   = useState(false);
  const [mqttClient, setMqttClient]     = useState();

  const [checked1, setChecked1] = useState(false);
  const [checked2, setChecked2] = useState(false);
  const [checked3, setChecked3] = useState(false);

  const [doorTime, setDoorTime] = useState("05/04/2023, 11:34:32");

  useEffect(() => {
    
    connectToAwsIot();
  
  },[]);  // the empty [] ensures only run once

  async function connectToAwsIot() {
    // This connection/function is only for publishing messages;
    // Subscriptions each get their own child object with separate connections.

    // mqtt clients require a unique clientId; we generate one below
    var clientId = 'iotcognito-cp-sampleapp-' + (Math.floor((Math.random() * 100000) + 1));

    // get credentials and, from them, extract key, secret key, and session token
    // Amplify's auth functionality makes this easy for us...
    var currentCredentials = await Auth.currentCredentials();
    var essentialCredentials = Auth.essentialCredentials(currentCredentials);

    // Create an MQTT client
    var newMqttClient = AWSIoTData.device({
      region: AWSConfiguration.region,
      host:AWSConfiguration.host,
      clientId: clientId,
      protocol: 'wss',
      maximumReconnectTimeMs: 8000,
      debug: true,
      accessKeyId:  essentialCredentials.accessKeyId,
      secretKey:    essentialCredentials.secretAccessKey,
      sessionToken: essentialCredentials.sessionToken
    });

    console.log('Publisher trying to connect to AWS IoT for clientId:', clientId);
    // On connect, update status
    newMqttClient.on('connect', function() {
      setIsConnected(true);
      console.log('Publisher connected to AWS IoT for clientId:', clientId);

      newMqttClient.subscribe("#");
      console.log('Connected to AWS IoT for clientId:', clientId);
      console.log(`Subscribed to "#"`);
    });

    // add event handler for received messages
    newMqttClient.on('message', function(topic, payload) {
      console.log(topic,"payload");
      const data = JSON.parse(payload.toString());
      if ( "Lock Door" === topic ) {
        setChecked1(data.status)
      }
      if ( "Dispense Water" === topic ) {
        setChecked2(data.status)
      }
      if ( "Dispense Food" === topic ) {
        setChecked3(data.status)
      }
      if ( "esp32/pub" === topic ) {
        setDoorTime(new Date().toLocaleString())
      }
    });

    // update state to track mqtt client
    setMqttClient(newMqttClient);

  }

  function removeSubscription(topic) {
    // This function is passed to child components
    setSubscribedTopics(arrayRemove(subscribedTopics,topic));
  }

  function handleSubscriptionRequest(e) {
    // stop submit button from refreshing entire page
    e.preventDefault();

    if (subscribedTopics.includes(desiredSubscriptionTopic)) {
      console.log(`You are already subscribed to topic '${desiredSubscriptionTopic}'!`);
    }
    else {
      setSubscribedTopics(prevTopics => [...prevTopics, desiredSubscriptionTopic]);
      console.log(`Subscribed to topic '${desiredSubscriptionTopic}'!`);
    }
  }

  function handlePublishRequest(e) {
    // stop submit button from refreshing entire page
    e.preventDefault();
  
    mqttClient.publish(desiredPublishTopic, desiredPublishMessage);
  
  }

    return (
      <>
      <Card>
        <b>API:</b> {isConnected ? "connected" : "Not connected"}
      </Card>
      <BlockTitle withBlock={false}>Status</BlockTitle>
      <Card>
        Door last opened: {doorTime}
      </Card>
      <Card>
        Food Bowl Weight: 100ml
      </Card>
      <BlockTitle withBlock={false}>Features</BlockTitle>
      <List strong inset>
        <ListItem
          label
          title="Lock Door"
          after={
            <Toggle
              component="div"
              className="-my-1"
              checked={checked1}
              onChange={() => {
                setChecked1(!checked1)
                if ( !checked1 ) {
                  mqttClient.publish( "Lock Door", `{ "status": 1 }` );
                } else {
                  mqttClient.publish( "Lock Door", `{ "status": 0 }` );
                }
              }}
            />
          }
        />
        <ListItem
          label
          title="Dispense Water"
          after={
            <Toggle
              component="div"
              className="-my-2"
              checked={checked2}
              onChange={() => {
                setChecked2(!checked2)
                if ( !checked2 ) {
                  mqttClient.publish( "Dispense Water", `{ "status": 1 }` );
                } else {
                  mqttClient.publish( "Dispense Water", `{ "status": 0 }` );
                }
              }}
            />
          }
        />
        <ListItem
          label
          title="Dispense Food"
          after={
            <Toggle
              component="div"
              className="-my-3"
              checked={checked3}
              onChange={() => {
                setChecked3(!checked3)
                if ( !checked3 ) {
                  mqttClient.publish( "Dispense Food", `{ "status": 1 }` );
                } else {
                  mqttClient.publish( "Dispense Food", `{ "status": 0 }` );
                }
              }}
            />
          }
        />
      </List>
      <BlockTitle withBlock={false}></BlockTitle>
      {/* <div className="MQTTDisplay"> */}
        {/* <div className=""> */}
          {/* <b>API status:</b> {isConnected ? "connected" : "Not connected"} */}
          {/* <br/><br/>  */}
          {/* <form onSubmit={handlePublishRequest}> */}
            {/* <b>Publish to Topic:</b>
            <br/>
            <input
              value={desiredPublishTopic}
              onChange={e => setDesiredPublishTopic(e.target.value)}
              placeholder="IoT Topic"
              type="text"
              name="desiredPublishTopic"
              required
            />
            <br/><br/> */}
            {/* <b>Outgoing Data:</b>
            <br/>
            <textarea
              value={desiredPublishMessage}
              onChange={e => setDesiredPublishMessage(e.target.value)}
              placeholder="IoT Topic"
              type="text"
              name="desiredPublishTopic"
              required
            />
            <br/><br/>
            <button className="bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded" type="submit">Send Data</button>   
            <br/>
          </form> */}
        {/* </div> */}
        {/* <br/> */}
        {/* <div className="">
          <form onSubmit={handleSubscriptionRequest} style={{display:'none'}}>
            <b>Subscribe to Topic:</b>
            <br/>
            <input
              value={desiredSubscriptionTopic}
              onChange={e => setDesiredSubscriptionTopic(e.target.value)}
              placeholder="IoT Topic"
              type="text"
              name="desiredSubscriptionTopic"
              required
            />
            <button type="submit">Subscribe</button>   
            <br/><br/>
          </form>
  
          <b>Incoming Data:</b> <br/>
          {subscribedTopics.map(topic => {
            return (<MQTTSubscription key={topic} topic={topic} removeSubscription={removeSubscription}/>)
          })}
        </div> */}
      {/* </div> */}
      </>
    );
  }

//######################################################################################
function MQTTSubscription(props) {

  const [isConnected, setIsConnected] = useState(false);
  const [mqttClient, setMqttClient]   = useState();
  const [messages, setMessages]       = useState([]);

  useEffect(() => {

    connectToAwsIot();

    return () => {
      // this gets called when component is destroyed...
      //https://github.com/mqttjs/MQTT.js/blob/master/README.md#end    
      console.log(`Ended subscription to '${props.topic}'...`);
    };

  },[]); // the "[]" causes this to execute just once

  async function connectToAwsIot() {

    // mqtt clients require a unique clientId; we generate one below
    var clientId = 'iotcognito-cp-sampleapp-' + (Math.floor((Math.random() * 100000) + 1));

    // get credentials and, from them, extract key, secret key, and session token
    // Amplify's auth functionality makes this easy for us...
    var currentCredentials = await Auth.currentCredentials();
    var essentialCredentials = Auth.essentialCredentials(currentCredentials);
    
    // Create an MQTT client
    var newMqttClient = AWSIoTData.device({
      region: AWSConfiguration.region,
      host:AWSConfiguration.host,
      clientId: clientId,
      protocol: 'wss',
      maximumReconnectTimeMs: 8000,
      debug: true,
      accessKeyId:  essentialCredentials.accessKeyId,
      secretKey:    essentialCredentials.secretAccessKey,
      sessionToken: essentialCredentials.sessionToken
     });
    console.log('Subscriber trying to connect to AWS IoT for clientId:', clientId);
    // On connect, update status
    newMqttClient.on('connect', function() {
      setIsConnected(true);
      newMqttClient.subscribe(props.topic);
      console.log('Connected to AWS IoT for clientId:', clientId);
      console.log(`Subscribed to ${props.topic}`);
    
    });

    // add event handler for received messages
    newMqttClient.on('message', function(topic, payload) {
      var myDate =      new Date().toLocaleDateString() + " " + new Date().toLocaleTimeString();
      var newMessage =  `${myDate} - topic '${topic}' - \n ${payload.toString()}`;
      setMessages(prevMessages => [...prevMessages, newMessage]);
      console.log(newMessage);
    });

    // update state to track mqtt client
    setMqttClient(newMqttClient);

  }

  function handleUnsubscribe(e) {
    // stop submit button from refreshing entire page
    e.preventDefault();

    // end subscription; I think this could be added to the return() of the useEffect(), as an "onUnmount" handler,
    // but I received an erropr when I tried it. I might be doing something wrong but for now, it works with the commands
    // below... 
    mqttClient.end(false); 
    setIsConnected(false);

    // remove subscription from parent component, thus killing this component...
    props.removeSubscription(props.topic);
  }

  return (
    <div className="MQTTSubscription">
      {/* ({isConnected ? "connected" : "not connected"}) */}
      {/* <form onSubmit={handleUnsubscribe}>
        <button type="submit">Unsubscribe</button>
      </form> */}
      <br/>

      {messages.map((message,index) => {
        return (
          <li key={index} className="markdown">
            {message}
          </li>
        );
      })}
    </div>
  );

}

export default MQTTDisplay;
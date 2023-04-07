import React from 'react';
import { withAuthenticator, Authenticator } from 'aws-amplify-react';
// import logo from './logo.svg';
// import './App.css';
import { App, Page, Navbar, Block, List, ListButton, Card, Link } from 'konsta/react';
import Title from './Title';

// import AuthDisplay from './AuthDisplay';
import MQTTDisplay from './MQTTDisplay'

function MyApp(props) {
  return (
    <App theme="ios">
      <Page>
        <Navbar title={<Title />} />
        <MQTTDisplay {...props} />
        <Card style={{textAlign: 'center'}}>
          <Link onClick={console.log("sign out")} style={{color:'rgb(259,59,48)'}}>Sign Out</Link>
        </Card>
      </Page>
    </App>
  );

}

export default withAuthenticator(MyApp);

var awsIotCoreConfiguration = {
    endpoint: 'iot.eu-west-1.amazonaws.com',
    region: 'eu-west-1',
    apiVersion: '2015-05-28',
    policy: '{"Version": "2012-10-17", "Statement": [{"Effect": "Allow", "Action": [ "iot:Subscribe" ], "Resource": ["arn:aws:iot:eu-west-1:499509086187:topicfilter/*"]},{"Effect": "Allow","Action": [ "iot:Connect" ],"Resource": ["arn:aws:iot:eu-west-1:499509086187:client/iotcognito-cp-sampleapp-*"] },{"Effect": "Allow","Action": [ "iot:Publish","iot:Receive" ],"Resource": ["arn:aws:iot:eu-west-1:499509086187:topic/*"]}]}'
  };

 export default awsIotCoreConfiguration;
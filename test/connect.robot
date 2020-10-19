*** Settings ***
| Library       | String
| Library       | MQTTLibrary
| Test Timeout  | 30 seconds


*** Variables ***
| ${broker.uri}     | 10.4.0.5
| ${broker.port}    | 1883
| ${client.id}      | test.client


| *Test Cases*
| Connect to a broker with default port and client id
| | ${mqttc}    | Connect   | ${broker.uri}
| | ${client_id} = | Decode Bytes To String | ${mqttc._client_id} | UTF-8
| | Should Be Empty         | ${client_id}  |
| | [Teardown]  | Disconnect

| Connect to a broker with default port and specified client id
| | ${mqttc}    | Connect   | ${broker.uri} | client_id=${client.id}
| | Should be equal as strings  | ${mqttc._client_id}   | ${client.id}
| | [Teardown]  | Disconnect

| Connect to a broker with specified port and client id
| | ${mqttc}    | Connect   | ${broker.uri} | ${broker.port}    | ${client.id}
| | Should be equal as strings  | ${mqttc._client_id}   | ${client.id}
| | [Teardown]  | Disconnect
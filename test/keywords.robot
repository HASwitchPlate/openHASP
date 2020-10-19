| *Settings*    | *Value*
| Library       | MQTTLibrary
| Library       | BuiltIn

| *Variables*       | *Value*
#| ${broker.uri}     | mqtt.eclipse.org
| ${broker.uri}     | 10.4.0.5
| ${broker.port}    | 1883
| ${client.id}      | mqtt.test.client
| ${topic}          | test/mqtt_test
| ${sub.topic}      | test/mqtt_test_sub

| *Keywords*    |
| Easy Connect
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${clean_session}=${true}
| | Connect     | ${broker.uri} | ${port}       | ${client.id}    | ${clean_session}

| Publish to MQTT Broker
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${clean_session}=${true}
| | ...         | ${topic}=${topic}             | ${message}=${EMPTY}
| | ...         | ${qos}=0                      | ${retention}=${false}
| | Connect     | ${broker.uri} | ${port}       | ${client.id}    | ${clean_session}
| | Publish     | ${topic}      | ${message}    | ${qos}    | ${retention}

| Publish to MQTT Broker and Disconnect
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${clean_session}=${true}
| | ...         | ${topic}=${topic}             | ${message}=${EMPTY}
| | ...         | ${qos}=0                      | ${retention}=${false}
| | Connect     | ${broker.uri} | ${port}       | ${client.id}    | ${clean_session}
| | Publish     | ${topic}      | ${message}    | ${qos}    | ${retention}
| | [Teardown]  | Disconnect

| Subscribe to MQTT Broker and Validate
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${topic}=${topic}
| | ...         | ${message}=${EMPTY}           | ${qos}=1
| | ...         | ${timeout}=1s
| | Connect     | ${broker.uri} | ${port}       | ${client.id}  | ${false}
| | Subscribe and Validate
| | ...         | ${topic}      | ${qos}        | ${message}        | ${timeout}
| | [Teardown]  | Disconnect

| Subscribe and Get Messages
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${topic}=${topic}
| | ...         | ${qos}=1                      | ${timeout}=1s
| | ...         | ${limit}=1
| | Connect     | ${broker.uri} | ${port}       | ${client.id}  | ${false}
| | @{messages} | Subscribe     | ${topic} | ${qos} | ${timeout}    | ${limit}
| | [Teardown]  | Disconnect
| | [Return]    | @{messages}

| Subscribe Async
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${topic}=${topic}
| | ...         | ${qos}=1                      | ${timeout}=0s
| | ...         | ${limit}=1
| | Connect     | ${broker.uri} | ${port}       | ${client.id}  | ${false}
| | Subscribe   | ${topic} | ${qos} | ${timeout}    | ${limit}

| Unsubscribe and Disconnect
| | [Arguments] | ${topic}=${topic}
| | Unsubscribe | ${topic}
| | [Teardown]  | Disconnect

| Unsubscribe Multiple and Disconnect
| | [Arguments] | @{topics}
| | FOR    | ${topic}    | IN    | @{topics}
| | | Unsubscribe    | ${topic}
| | END
| | [Teardown]  | Disconnect

| Subscribe and Unsubscribe
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${topic}=${topic}
| | ...         | ${qos}=1                      | ${timeout}=1s
| | ...         | ${limit}=1
| | Connect     | ${broker.uri} | ${port}       | ${client.id}  | ${false}
| | @{messages} | Subscribe     | ${topic} | ${qos} | ${timeout}    | ${limit}
| | Unsubscribe | ${topic}
| | [Teardown]  | Disconnect
| | [Return]    | @{messages}

| Listen and Get Messages
| | [Arguments] | ${topic}=${topic}   | ${timeout}=1s
| | ...         | ${limit}=1
| | @{messages} | Listen     | ${topic} | ${timeout}    | ${limit}
| | [Return]    | @{messages}
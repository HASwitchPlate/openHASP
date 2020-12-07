| *Settings*    | *Value*
| Resource      | keywords.robot
| Test Timeout  | 30 seconds

| *Test Cases*
| Publish a non-empty message
| | ${time}     | Get Time      | epoch
| | ${message}  | Catenate      | test message  | ${time}
| | Publish to MQTT Broker and Disconnect       | message=${message}

| Publish an empty message
| | Publish to MQTT Broker and Disconnect

| Publish a message with QOS 1 and validate that the message is received
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | ${message}  | Set Variable  | subscription test message
| |  Run Keyword And Expect Error               | The expected payload didn't arrive in the topic
| | ... | Subscribe to MQTT Broker and Validate | client.id=${client}   | topic=${topic}        | message=${message}
| | Publish to MQTT Broker and Disconnect       | topic=${topic}        | message=${message}    | qos=1
| | Subscribe to MQTT Broker and Validate       | client.id=${client}   | topic=${topic}        | message=${message}

| Publish multiple messages and confirm that validation succeeds only after correct message is published
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | ${message}  | Set Variable  | subscription test message
| |  Run Keyword And Expect Error               | The expected payload didn't arrive in the topic
| | ... | Subscribe to MQTT Broker and Validate | client.id=${client}   | topic=${topic}        | message=${message}
| | Publish to MQTT Broker and Disconnect       | topic=${topic}        | message=message1      | qos=1
| | Publish to MQTT Broker and Disconnect       | topic=${topic}        | message=message2      | qos=1
| |  Run Keyword And Expect Error               | The expected payload didn't arrive in the topic
| | ... | Subscribe to MQTT Broker and Validate | client.id=${client}   | topic=${topic}        | message=${message}
| | Publish to MQTT Broker and Disconnect       | topic=${topic}        | message=${message}    | qos=1
| | Publish to MQTT Broker and Disconnect       | topic=${topic}        | message=message3      | qos=1
| | Publish to MQTT Broker and Disconnect       | topic=${topic}        | message=message4      | qos=1
| | Subscribe to MQTT Broker and Validate       | client.id=${client}   | topic=${topic}        | message=${message}

| Publish an empty message with QOS 1 and validate
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| |  Run Keyword And Expect Error               | The expected payload didn't arrive in the topic
| | ... | Subscribe to MQTT Broker and Validate | client.id=${client}     | topic=${topic}
| | Publish to MQTT Broker and Disconnect       | topic=${topic}          | qos=1
| | Subscribe to MQTT Broker and Validate       | client.id=${client}     | topic=${topic}

| Publish and validate with regular expression
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | ${message}  | Set Variable  | subscription test message
| | ${regex}    | Set Variable  | ^subscription [test]{4} message$
| |  Run Keyword And Expect Error               | The expected payload didn't arrive in the topic
| | ... | Subscribe to MQTT Broker and Validate | client.id=${client}   | topic=${topic}        | message=whatever
| | Publish to MQTT Broker and Disconnect       | topic=${topic}        | message=${message}    | qos=1
| | Subscribe to MQTT Broker and Validate       | client.id=${client}   | topic=${topic}        | message=${regex}

| Subscribe for the first time and validate that no messages are received
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | @{messages} | Subscribe and Get Messages    | client.id=${client}   | topic=${topic}    | timeout=5s
| | LOG         | ${messages}
| | Length Should Be    | ${messages}   | 0

| Subscribe, publish 1 message and validate it is received
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | Subscribe and Get Messages  | client.id=${client}   | topic=${topic}
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message      | qos=1
| | @{messages} | Subscribe and Get Messages    | client.id=${client}   | topic=${topic}
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 1
| | Should Be Equal As Strings  | ${messages}[0]    | test message

| Subscribe with no limit, publish multiple messages and validate they are received
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | Subscribe and Get Messages  | client.id=${client}   | topic=${topic}
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message1 | qos=1
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message2 | qos=1
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message3 | qos=1
| | @{messages} | Subscribe and Get Messages    | client.id=${client}   | topic=${topic} | limit=0
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 3
| | Should Be Equal As Strings  | ${messages}[0]    | test message1
| | Should Be Equal As Strings  | ${messages}[1]    | test message2
| | Should Be Equal As Strings  | ${messages}[2]    | test message3

| Subscribe with limit
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | Subscribe and Get Messages  | client.id=${client}   | topic=${topic}
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message1 | qos=1
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message2 | qos=1
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message3 | qos=1
| | @{messages} | Subscribe and Get Messages    | client.id=${client}   | topic=${topic} | limit=1
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 1
| | Should Be Equal As Strings  | ${messages}[0]    | test message1
| | @{messages} | Subscribe and Get Messages    | client.id=${client}   | topic=${topic} | limit=2
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 2
| | Should Be Equal As Strings  | ${messages}[0]    | test message2
| | Should Be Equal As Strings  | ${messages}[1]    | test message3

| Unsubscribe and validate no messages are received
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | Subscribe and Get Messages  | client.id=${client}   | topic=${topic}
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message1 | qos=1
| | @{messages} | Subscribe and Unsubscribe | client.id=${client}   | topic=${topic}
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message2 | qos=1
| | @{messages} | Subscribe and Get Messages    | client.id=${client}   | topic=${topic}
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 0

| Publish and Subscribe to a broker that requires username, password authentication
| | [Tags]      | auth
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test
| | ${message}  | Set Variable  | subscription test message
| | Set username and password   | authuser1     | password1
| | Run Keyword And Expect Error                | The expected payload didn't arrive in the topic
| | ... | Subscribe to MQTT Broker and Validate | broker.uri=127.0.0.1  | port=11883    | client.id=${client}
| | ... | topic=${topic}        | message=${message}
| | Connect     | 127.0.0.1     | 11883
| | Publish     | ${topic}      | test message with username and password   | qos=1
| | Subscribe to MQTT Broker and Validate
| | ...         | broker.uri=127.0.0.1  | port=11883    | client.id=${client}   | topic=${topic}        | message=test message with username and password
| | [Teardown]  | Disconnect

| Publish to a broker that requires authentication with invalid password
| | [Tags]      | auth
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test
| | Set username and password   | authuser1     | invalidpwd
| | Run Keyword and expect error    | The client disconnected unexpectedly
| | ...         | Connect     | 127.0.0.1     | 11883          | ${client}
| | [Teardown]  | Disconnect

| Subscribe async, publish 1 message, listen for and validate it is received
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | Subscribe Async | client.id=${client}   | topic=${topic}
| | Publish to MQTT Broker   | topic=${topic}    | message=test message      | qos=1
| | @{messages} | Listen and Get Messages    | topic=${topic}
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 1
| | Should Be Equal As Strings  | ${messages}[0]    | test message
| | [Teardown]  | Unsubscribe and Disconnect  | ${topic}

| Subscribe async, publish several messages, listen for and validate they are received
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | Subscribe Async | client.id=${client}   | topic=${topic}
| | Publish to MQTT Broker   | topic=${topic}    | message=test message1      | qos=1
| | Publish to MQTT Broker   | topic=${topic}    | message=test message2      | qos=1
| | Publish to MQTT Broker   | topic=${topic}    | message=test message3      | qos=1
| | @{messages} | Listen and Get Messages    | topic=${topic} | limit=0
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 3
| | Should Be Equal As Strings  | ${messages}[0]    | test message1
| | Should Be Equal As Strings  | ${messages}[1]    | test message2
| | Should Be Equal As Strings  | ${messages}[2]    | test message3
| | [Teardown]  | Unsubscribe and Disconnect  | ${topic}

| Subscribe async to multiple topics, publish several messages, listen for them and validate they are received
| | ${time}     | Get Time      | epoch
| | ${client1}  | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${client2}  | Catenate      | SEPARATOR=.   | robot.mqtt | ${time+1}
| | ${topic1}   | Set Variable  | test/mqtt_test_sub1
| | ${topic2}   | Set Variable  | test/mqtt_test_sub2
| | Subscribe Async | client.id=${client1}   | topic=${topic1}
| | Subscribe Async | client.id=${client2}   | topic=${topic2}
| | Publish to MQTT Broker   | topic=${topic2}    | message=test message1      | qos=1
| | Publish to MQTT Broker   | topic=${topic1}    | message=test message2      | qos=1
| | Publish to MQTT Broker   | topic=${topic2}    | message=test message3      | qos=1
| | @{messages1} | Listen and Get Messages    | topic=${topic1} | limit=0
| | @{messages2} | Listen and Get Messages    | topic=${topic2} | limit=0
| | LOG         | ${messages1}
| | LOG         | ${messages2}
| | Length Should Be            | ${messages1}       | 1
| | Should Be Equal As Strings  | ${messages1}[0]    | test message2
| | Length Should Be            | ${messages2}       | 2
| | Should Be Equal As Strings  | ${messages2}[0]    | test message1
| | Should Be Equal As Strings  | ${messages2}[1]    | test message3
| | [Teardown]  | Unsubscribe Multiple and Disconnect  | ${topic1}    | ${topic2}

| Listen immediately after Subscribe and validate message is received
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | test/mqtt_test_sub
| | Subscribe and Get Messages  | client.id=${client}   | topic=${topic}
| | Publish to MQTT Broker and Disconnect   | topic=${topic}    | message=test message      | qos=1
| | Subscribe Async             | client.id=${client}   | topic=${topic}
| | @{messages}= | Listen       | topic=${topic} | limit=10 | timeout=5
| | Should Be Equal As Strings  | ${messages}[0]    | test message
| | [Teardown]  | Unsubscribe and Disconnect | ${topic}
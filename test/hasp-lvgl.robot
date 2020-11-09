| *Settings*    | *Value*
| Resource      | keywords.robot
| Test Timeout  | 240 seconds

| *Keywords*
| Test Property
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${clean_session}=${true}
| | ...         | ${property}=${property}       | ${data}=${data}
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | hasp/plate35/command
| | ${restopic} | Set Variable  | hasp/plate35/state/json
| | ${qos}      | Set Variable  | 1
| | ${message}  | Set Variable  | ${property}=${data}
| | ${result}   | Set Variable  | {"${property}":"${data}"}
| | Sleep  | .01s
| | Subscribe Async  | client.id=${client}   | topic=${restopic}
| | Connect     | ${broker.uri} | ${port}       | ${client.id}    | ${clean_session}
| | Publish | ${topic}    | ${message}   | 1
| | Publish | ${topic}    | ${property}  | 1
| | log to console | ${result}
| | @{messages} | Listen and Get Messages    |  topic=${restopic} | limit=1 | timeout=1.5
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 1
| | Should Be Equal As Strings  | ${messages}[0]    | ${result}

| Test Page
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${clean_session}=${true}
| | ...         | ${property}=${property}       | ${data}=${data}
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | hasp/plate35/command
| | ${restopic} | Set Variable  | hasp/plate35/state/page
| | ${qos}      | Set Variable  | 1
| | ${message}  | Set Variable  | ${property}=${data}
| | Subscribe Async  | client.id=${client}   | topic=${restopic}
| | Connect     | ${broker.uri} | ${port}       | ${client.id}    | ${clean_session}
| | Publish | ${topic}    | ${message}   | 1
| | Publish | ${topic}    | ${property}  | 1
| | @{messages} | Listen and Get Messages    |  topic=${restopic} | limit=1 | timeout=1
| | LOG         | ${messages}
| | Length Should Be            | ${messages}       | 1
| | Should Be Equal As Strings  | ${messages}[0]    | ${data}

| Hasp Command
| | [Arguments] | ${broker.uri}=${broker.uri}   | ${port}=${broker.port}
| | ...         | ${client.id}=${client.id}     | ${clean_session}=${true}
| | ...         | ${property}=${property}       | ${data}=${data}
| | ${time}     | Get Time      | epoch
| | ${client}   | Catenate      | SEPARATOR=.   | robot.mqtt | ${time}
| | ${topic}    | Set Variable  | hasp/plate35/command/${property}
| | ${restopic} | Set Variable  | hasp/plate35/state/page
| | ${qos}      | Set Variable  | 1
| | ${message}  | Set Variable  | ${data}
| | Connect     | ${broker.uri} | ${port}       | ${client.id}    | ${clean_session}
| | Publish | ${topic}    | ${message}   | 1

| *Test Cases*

| Test Color Picker\n
| | ${obj}      | Set Variable  | p[1].b[4]
| | Test Page | property=page | data=1
| | Hasp Command | property=clearpage | data=1
| | Hasp Command | property=jsonl | data={"page":1,"id":4,"objid":20}
#| | Test Property | property=${obj}.txt | data=ABC
#| | Test Property | property=${obj}.txt | data=1234
| | Test Property | property=${obj}.x | data=50
| | Test Property | property=${obj}.x | data=60
| | Test Property | property=${obj}.y | data=70
| | Test Property | property=${obj}.y | data=80
| | Test Property | property=${obj}.w | data=80
| | Test Property | property=${obj}.w | data=100
| | Test Property | property=${obj}.h | data=80
| | Test Property | property=${obj}.h | data=100
| | Test Property | property=${obj}.hidden | data=1
| | Test Property | property=${obj}.hidden | data=0
| | Test Property | property=${obj}.vis | data=0
| | Test Property | property=${obj}.vis | data=1
| | Test Property | property=${obj}.enabled | data=0
| | Test Property | property=${obj}.enabled | data=1
| | Test Property | property=${obj}.opacity | data=0
| | Test Property | property=${obj}.opacity | data=64
| | Test Property | property=${obj}.opacity | data=192
| | Test Property | property=${obj}.opacity | data=255
#| | Test Property | property=${obj}.rect | data=1
#| | Test Property | property=${obj}.rect | data=0
| | Test Property | property=${obj}.val | data=50
| | Test Property | property=${obj}.val | data=60
| | Test Property | property=${obj}.val | data=70
| | Test Property | property=${obj}.val | data=80

| Test Text Field\n
| | ${obj}      | Set Variable  | p[1].b[1]
| | Test Page | property=page | data=1
| | Hasp Command | property=clearpage | data=1
| | Hasp Command | property=jsonl | data={"page":1,"id":1,"objid":12}
| | Test Property | property=${obj}.txt | data=ABC
| | Test Property | property=${obj}.txt | data=123
| | Test Property | property=${obj}.x | data=20
| | Test Property | property=${obj}.x | data=10
| | Test Property | property=${obj}.y | data=20
| | Test Property | property=${obj}.y | data=10
#| | Test Property | property=${obj}.w | data=80
#| | Test Property | property=${obj}.w | data=75
#| | Test Property | property=${obj}.h | data=36
#| | Test Property | property=${obj}.h | data=18
| | Test Property | property=${obj}.hidden | data=1
| | Test Property | property=${obj}.hidden | data=0
| | Test Property | property=${obj}.vis | data=0
| | Test Property | property=${obj}.vis | data=1
| | Test Property | property=${obj}.enabled | data=0
| | Test Property | property=${obj}.enabled | data=1
| | Test Property | property=${obj}.opacity | data=0
| | Test Property | property=${obj}.opacity | data=64
| | Test Property | property=${obj}.opacity | data=192
| | Test Property | property=${obj}.opacity | data=255


| Test Button\n
| | ${obj}      | Set Variable  | p[1].b[1]
| | Test Page | property=page | data=1
| | Hasp Command | property=clearpage | data=1
| | Hasp Command | property=jsonl | data={"page":1,"id":1,"objid":10}
#| | Test Property | property=${obj}.txt | data=ABC
#| | Test Property | property=${obj}.txt | data=1234
| | Test Property | property=${obj}.x | data=20
| | Test Property | property=${obj}.x | data=10
| | Test Property | property=${obj}.y | data=20
| | Test Property | property=${obj}.y | data=10
| | Test Property | property=${obj}.w | data=80
| | Test Property | property=${obj}.w | data=75
| | Test Property | property=${obj}.h | data=36
| | Test Property | property=${obj}.h | data=18
| | Test Property | property=${obj}.hidden | data=1
| | Test Property | property=${obj}.hidden | data=0
| | Test Property | property=${obj}.vis | data=0
| | Test Property | property=${obj}.vis | data=1
| | Test Property | property=${obj}.enabled | data=0
| | Test Property | property=${obj}.enabled | data=1
| | Test Property | property=${obj}.opacity | data=0
| | Test Property | property=${obj}.opacity | data=64
| | Test Property | property=${obj}.opacity | data=192
| | Test Property | property=${obj}.opacity | data=255
| | Test Property | property=${obj}.toggle | data=0
| | Test Property | property=${obj}.toggle | data=1
| | Test Property | property=${obj}.val | data=0
| | Test Property | property=${obj}.val | data=1
| | Test Property | property=${obj}.val | data=2
| | Test Property | property=${obj}.val | data=3

| Test Slider\n
| | ${obj}      | Set Variable  | p[1].b[4]
| | Test Page | property=page | data=1
| | Hasp Command | property=clearpage | data=1
| | Hasp Command | property=jsonl | data={"page":1,"id":4,"objid":30}
#| | Test Property | property=${obj}.txt | data=ABC
#| | Test Property | property=${obj}.txt | data=1234
| | Test Property | property=${obj}.x | data=20
| | Test Property | property=${obj}.x | data=10
| | Test Property | property=${obj}.y | data=20
| | Test Property | property=${obj}.y | data=10
| | Test Property | property=${obj}.w | data=80
| | Test Property | property=${obj}.w | data=75
| | Test Property | property=${obj}.h | data=36
| | Test Property | property=${obj}.h | data=18
| | Test Property | property=${obj}.hidden | data=1
| | Test Property | property=${obj}.hidden | data=0
| | Test Property | property=${obj}.vis | data=0
| | Test Property | property=${obj}.vis | data=1
| | Test Property | property=${obj}.enabled | data=0
| | Test Property | property=${obj}.enabled | data=1
| | Test Property | property=${obj}.opacity | data=0
| | Test Property | property=${obj}.opacity | data=64
| | Test Property | property=${obj}.opacity | data=192
| | Test Property | property=${obj}.opacity | data=255
| | Test Property | property=${obj}.max | data=200
| | Test Property | property=${obj}.min | data=100
| | Test Property | property=${obj}.min | data=50
| | Test Property | property=${obj}.max | data=150
| | Test Property | property=${obj}.val | data=50
| | Test Property | property=${obj}.val | data=60
| | Test Property | property=${obj}.val | data=70
| | Test Property | property=${obj}.val | data=80

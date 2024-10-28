```mermaid
sequenceDiagram
MCP ->> CCP: *Sends Command in UDP Packet*
CCP ->> MCP: *Sends Ackknowledgement Packet*
activate CCP
CCP ->> CEP: *Sends Comand in UDP Packet, Correctly Formatted*
deactivate CCP
CEP ->> CCP: *Sends Ackknowledgement Packet*
activate CEP
CEP ->> Carraige Companants/sensors: *Alter the state of the Carriage (Speed/Doors/LED's)*
deactivate CEP
activate Carraige Companants/sensors
loop Every 2 Seconds
Carraige Companants/sensors ->> CEP: *Sends Positional data & Speed data*
deactivate Carraige Companants/sensors
CEP ->> Carraige Companants/sensors: *Sends Ackknowledgement Packet*
CEP ->> CCP: *Passes Positional data & Speed Data*
CCP ->> CEP: *Sends Ackknowledgement Packet*
CCP ->> MCP: *Passes Positional data & Speed Data*
end
```

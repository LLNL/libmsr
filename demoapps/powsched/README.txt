------ Environmental Variables ------
EXPOSE_MASTER_HOST - the host running the root Cache instance
EXPOSE_MASTER_PORT - the port number of the root Cache instance


------ Startup ------
Run the start.sh script.

start.sh starts the root Cache instance, loads the schema, and then waits on the node state for the root node to change.


------ Applications ------
daemonreg -
This tool installs automata that are to run indefinitely and that do not
return data to the registering process.

power_actuator -
This POWsched component attempts to apply updated allocation settings to
individual nodes. power_actuator conducts a sanity check before actually
applying the settings and will default to a naive fairshare allocation if the
power targets exceed the global bound.

power_monitor -
This POWsched component produces power measurements and publishes them via
expose.

powsched -
This is the main scheduler. Powsched requests power readings be collected by
expose and periodically inserts new power settings. After settings have been
inserted, powsched signals the power_actuator to apply the settings.

haltwait -
This process connects to the Cache instance and watches the newnodestate to
detect when the state changes to something less than 0. This tool is useful
for scripts that must not exit until a global kill signal occurs.

shutdown -
This process connects to the Cache instance and changes the node state to -1.
Paired with haltwait, shutdow allows coordinated shutdown of loosely coupled
scripts.

------ Tables ------
cluster -
A persistent table showing the most current node state information
node, state

newnodestate -
A topic used to notify the system of node state updates
node, state

availmetrics - 
A persistent table showing the metrics and associated nodes available
pk (node-metric), node, metric, desc

metrictoggles - 
A topic used to request metrics be published
node, metric, onoff (0 off), intervalms

PowerIn -
Per node power reading events

poweractuals -
Latest per node power readings

powertargets -
per node desired power settings


------ Automata ------
powerstate.gapl
Converts power reading events into power state information

stateupdate.gapl
Converts node state events into node state information


------ Conventions ------
The node name is the name returned by the C gethostname() function. This name was selected because it should be available and the same for all processes on the node.

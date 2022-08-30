# Glossary

### Terms/functions etc that are commonly used across various classes will be documented here

_landUnitData : Used to get reference to pools, state variables associated with a module and create and submit carbon transfers (createStockOperation/createProportionalOperation/submitOperation)

### Events 

The FLINT framework uses publish/subscribe messaging to control the lifecycle of a simulation. Module code can subscribe to these events to perform work at different stages. See CBMSequencer and CBMSpinupSequencer (notificationCenter.postNotification) for when these events are fired in a GCBM simulation, as well as moja::flint::SpatialTiledLocalDomainController for the framework-level events like onSystemInit.

There are various events defined in the file [cbmmodulebase.h](https://github.com/moja-global/moja.canada/blob/develop/Source/moja.modules.cbm/include/moja/modules/cbm/cbmmodulebase.h)

| Event | Description |
| :-----------: | :---: |
| `onSystemInit` | Fired once when the framework starts up. At this point, modules have access to member variables initialized in the configuration step (flint::IModule::configure), but not pools or data variables | 
| `onSystemShutDown` | Fired once when the framework shuts down |
| `onLocalDomainInit` | Event is fired once per thread at the start of the simulation - modules typically subscribe to this event in order to store references to pools and variables. Non-spatial variable data is also available at this stage, i.e. variables with fixed values in the JSON configuration files |
| `onLocalDomainShutDown` | Fired once per thread when the simulation has ended |
| `onTimingInit` | Fired once for each pixel just before simulation for that pixel begins. This event is typically used to initialize the starting state for the pixel |
| `onTimingStep` | Invoked every timestep for a pixel’s simulation. Simulation is performed for the whole range of timesteps for a single land unit (pixel) before the framework moves on to the next land unit |
| `onOutputStep` | Fired at the end of each timestep for a land unit (pixel), typically for output modules to subscribe to |
| `onPreTimingSequence` | This event runs just before onTimingInit, for modules that need to do some work before that event |
| `onTimingPrePostInit` | This event is fired by some of the other FLINT framework sequencers (i.e. CalendarSequencer) just after TimingInit, but is unused in GCBM; it is overridden in CBMModuleBase just for completeness |
| `onTimingPostInit` | Fired just after TimingInit, before simulation begins for a pixel |
| `onTimingPreEndStep` | Just before the end of the current timing step |
| `onTimingEndStep` | When the current time step ends |
| `onTimingPostStep` | Fired after the current timing step ends |
| `onPrePostDisturbanceEvent` | Fired just before disturbance events in other non-GCBM FLINT framework sequencers. GCBM only uses this event in some of the peatland modules, which manually fire it before peatland-specific disturbance events |
| `onDisturbanceEvent` | Fired when a disturbance event occurs along with an optional “payload” argument. In the GCBM, the payload contains the disturbance type and the disturbance matrix, which defines all the carbon pool transfers resulting from the disturbance. Other modules have a chance to modify the disturbance matrix before the carbon pool transfers are processed; for example, the peatland modules watch for fire disturbances and add peatland carbon pool transfers to the fire matrix, which doesn’t normally include them |
| `onError` | Fired on the occurrence of an error | 
| `onPostNotification` | Not used |





[Back to Home](README.md)
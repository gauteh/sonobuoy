# Sonobuoy for measuring the Microseismicity of the Gakkel Ridge

This project was used for a [small seismic station network monitoring earthquakes below the Arctic Ocean at the Gakkel Ridge at 85N](https://bora.uib.no/handle/1956/6799). The results are described in the [thesis](https://github.com/gauteh/sonobuoy-gautebuoy/blob/master/Gaute%20Hope%20-%202013%20-%20Exploring%20the%20Microseismicity%20of%20the%20Gakkel%20Ridge%20from%20Arctic%20Sea%20Ice.pdf).

Five buoys were deployed. They operated using an internal network, communicating with a base-station which continuously monitored and recorded the incoming data. The data was also recorded on an internal SD-card.

An [Olimexino-STM32](https://www.olimex.com/Products/Duino/STM32/OLIMEXINO-STM32/) (a [Maple-clone](http://www.leaflabs.com/about-maple/) ) was used for the main board.

Refer to the thesis for a full description of the design. The code for the device and the base-station can be found in this repository.

## Reference

> Hope, Gaute. "Exploring the microseismicity of the Gakkel Ridge from Arctic 
> sea ice." (2013). [https://bora.uib.no/handle/1956/6799](https://bora.uib.no/handle/1956/6799)

## Abstract

> Monitoring of microseismicity is an important tool to understand the magmatism,
> tectonics and hydrothermal circulation of a mid-oceanic ridge. In the presence
> of sea ice, use of ocean bottom seismometers is not practical, but several
> experiments with small networks of seismometers drifting with the sea ice have
> previously been completed successfully. The goal of this project was to build
> on the earlier experiments and develop a method for monitoring the
> microseismicity above the Gakkel Ridge by developing seismic stations tailored
> for a small network on the Arctic sea ice. The new equipment has been taken
> from concept through development, building and deployment with data acquisition
> to data analysis. A 3D location algorithm was developed in order to locate a
> sample of the recorded events. Five prototype seismic stations were designed,
> built and tested in a small network on the sea ice. The stations had individual
> timing and positioning, and were connected in a wireless network for real time
> transmission of position, status and data. The network was deployed on the sea
> ice above the Gakkel Ridge at 85N during the FRAM-2012 expedition to the Arctic
> Ocean with the hovercraft R/H Sabvabaa in July - September 2012. The network
> drifted over the rift valley along with the ice and two re-deployments were
> made, totalling 23 days of recording. The last of the three deployments (7
> days) was the most successful, with three of the buoys fully operational.
> Although technical challenges had to be overcome due to the prototype nature of
> the equipment, it proved practical and functional. The last deployment was
> analyzed and 41 events were identified. To account for the rough bathymetry at
> the Gakkel Ridge, a 3D grid search based location algorithm was developed, with
> which three of the events were located. The prototype and the experiences from
> the deployment are presented together with an overview of the recorded data,
> the location algorithm, results from data analysis with the three event
> locations and interpretations.


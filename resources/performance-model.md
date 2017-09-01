# DAG-based Performance Model


## Background
Approaches to analyzing the performance of applications range from high-level analytical modelsthat provide coarse estimates of the performance of a small set of numerical kernels runningon a simple model of a processing platform, to sophisticated tools that provide accurateperformance estimations or measurements of actual execution on a givenplatform.


Similar to these approaches, wewant a model that captures the interaction of applications’ properties and key resources of modernmicroarchitectures, and provides reasonable estimates of performance on modern processors. Incontrast to existing approaches, we target at inherent properties from the computation DAG, e.g.,W, Q, and detailed performance data over the course of the execution.


Fig. 1.3 illustrates our proposed approach to bridging the gap between high-level analyticalmodels and detailed microarchitectural simulation. 




![alt text](https://raw.githubusercontent.com/caparrov/ERM-4.0.1/master/resources/perf-model-overview.png "")


As shown in the figure, our work builds onprior high-level DAG-based analytical models and thus is a DAG-based *model approach*. Ourmodel, however, considers a much more comprehensive set of platform parameters to model theprocessor, such as out-of-order (OoO) execution buffers, latency and bandwidths of a multi-levelmemory hierarchy, or instruction fetch bandwidth. Thus, it achieves more accurate performanceestimates and yields deeper insights into the interaction of the DAG with the modeled hardwareresources, without the need to use a full-fledged simulator or access to a given platform.Due to the increased complexity of our microarchitectural model, the performance is not estimatedby formulas as in classical methods, but by a tool that both dynamically generates andschedules the DAG on the modeled microarchitecture. This tool, similarly to microarchitecturalsimulators, models the execution of an application on a cycle-by-cycle basis; in contrast tomicroarchitectural simulators, it only models inherent properties of the application, and highlevelfeatures of a microarchitecture. For example, it considers floating-point computations andmemory accesses, but does not model machine-specific address calculation operations, or callingconventions. As another example, it considers a multi-level cache hierarchy, but does not modelthe details of cache associativity or the virtual memory system.

// SUmmarize by ERM




## Proposed DAG-based performance model


We propose a DAG-based performance model that considers the execution of the computationDAG on the abstraction of a single-core processor that exhibits a wider range of platform parametersthan those in Fig. 2.2 aimed at capturing the main features of the modern microarchitecturesin Figs. 2.3–2.5. Our goal is to achieve higher accuracy and deeper insights into the interactionof the application with the modeled hardware resources than prior classical models. Due to theincreased complexity of the microarchitectural model, our approach requires a simulation tool toboth dynamically generate and analyze the DAG.



### Microarchitecture model
Fig. 2.7 shows the microarchitecture model used in our proposed analysis. It extends the externalmemorymodel from Fig. 2.2 with some of the resources of superscalar microarchitectures [61] that



### DAG scheduling

We use Tomasulo’s greedy algorithm [67] to schedule the dynamically unfolding computationonto the resources listed in Table 2.2. We adapt the algorithm to incorporate the additional OoOexecution buffers defined above and to obey the additional constraints on the memory reorderingimposed by the platform’s memory model.


ERM is based on a novel DAG-based performance model that extendsclassical analytical high-level DAG-based models to consider a more detailed abstractionof a processor core



## Properties of the scheduled DAG
<!---
![alt text](https://raw.githubusercontent.com/caparrov/ERM-4.0.1/master/resources/images/livermore-kernel-23.png width="48")
![alt text](https://raw.githubusercontent.com/caparrov/ERM-4.0.1/master/resources/images/scheduled-DAG.png "")
-->

<a href="url"><img src="https://raw.githubusercontent.com/caparrov/ERM-4.0.1/master/resources/images/livermore-kernel-23.png" align="left" height="100" width="100" ></a>


<a href="url"><img src="https://raw.githubusercontent.com/caparrov/ERM-4.0.1/master/resources/images/scheduled-DAG.png" align="left" height="100" width="100" ></a>





The result of the previous analysis is a scheduled DAG as shown in Fig. 2.10(b) for the numericalkernel in Fig. 2.10(a). The DAG contains five different types of nodes and, in contrast to theDAGs shown in Fig. 2.1, the nodes’ execution cycle is determined by both data dependencesand the input microarchitectural constraints. For example, the execution of node (4) is delayeddue to memory bandwidth availability (only two nodes of that type can be executed per cycleassuming L1-ld = 2 loads/cycle), and the length of node (3) is five cycles due to the latency ofthe corresponding functional unit (mul = 5 cycles). In the following, we define the properties of


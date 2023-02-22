[![GitHub Action
Status](https://github.com/IntelligentRoboticsLabs/ros2_planning_system/workflows/master/badge.svg)](https://github.com/Docencia-fmrico/patrolling-roscon)

# PATROLLING BEHAVIOR BY ROSCON

## Problema a Resolver

En grupos de 4, haced una aplicación en ROS 2 que haga que un robot patrulle a lo largo de al menos 4 waypoints.

* Debes usar Behavior Trees y Nav2.
* Debe navegar en el mundo de Gazebo que prefieras, que hayas mapeado previamente.
* El robot debe realizar alguna acción cuando llegue a un waypoint (mostrar mensaje, girar sobre sí mismo,...)

El robot debe funcionar en el robot Tiago simulado.

## INTEGRANTES

* Oriana Acosta
* Jaime Avilleira
* Almudena Moreno
* Carlota Vera

## OBJETIVO

Ir a un número determinado de waypoints ubicados en el mapa haciendo uso de Nav2.

## COMPORTAMIENTO

La solución del problema se plantea en este Behavior Tree:

  ![árbol](https://github.com/Docencia-fmrico/patrolling-roscon/blob/main/BT.png)

Cuyo comportamiento es:

### 1.  [GetWayPoint](https://github.com/Docencia-fmrico/patrolling-roscon/blob/main/patrolling_bt/src/patrolling_bt/GetWaypoint.cpp)

Obtiene las coordenadas del siguiente waypoint siempre que esté dentro del rango. Si ya ha ido a todos los waypoints definido, se dirige a la posición final. Devuelve 'failure' cuando ya ha alcanzado la posición final.

![code1](https://github.com/Docencia-fmrico/patrolling-roscon/blob/readme/Screenshot%20from%202023-02-22%2001-23-03.png)

![code2](https://github.com/Docencia-fmrico/patrolling-roscon/blob/readme/Screenshot%20from%202023-02-22%2001-22-28.png)

Los waypoints elegidos se pueden ver en la siguiente imagen:

![mapa](https://github.com/Docencia-fmrico/patrolling-roscon/blob/readme/waypoints.png)

### 2. [Move](https://github.com/Docencia-fmrico/patrolling-roscon/blob/main/patrolling_bt/src/patrolling_bt/Move.cpp)

Recibe como input las coordenadas del siguiente waypoint y las pasa a Nav2. Espera el resultado de la navegación y cuando llega a la posición indicada, devuelve success. Mientras, devuelve running.

![code3](https://github.com/Docencia-fmrico/patrolling-roscon/blob/readme/Screenshot%20from%202023-02-22%2001-22-05.png)

### 3. [Patrol](https://github.com/Docencia-fmrico/patrolling-roscon/blob/main/patrolling_bt/src/patrolling_bt/Patrol.cpp)

Gira durante 15 segundos y mientras devuelve running. Una vez pasados estos segundos, devuelve success y vuelve a empezar el sequence. 

![code4](https://github.com/Docencia-fmrico/patrolling-roscon/blob/readme/Screenshot%20from%202023-02-22%2001-21-29.png)


## [TESTS](https://github.com/Docencia-fmrico/patrolling-roscon/blob/readme/patrolling_bt/tests/bt_action_test.cpp)

Se han diseñado una serie de test que , entre otras tareas, comprueban si son correctos los waypoints que se van a buscar.

## DEMOSTRACIÓN

[![VIDEO](https://i.ytimg.com/vi/Z9ddVyDX9ZY/hqdefault.jpg?sqp=-oaymwE2CPYBEIoBSFXyq4qpAygIARUAAIhCGAFwAcABBvABAfgB_gmAAtAFigIMCAAQARhlIGUoZTAP&rs=AOn4CLBRwjkDoGL4b-hhY9UAjVcOTKInlA)]

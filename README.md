[![GitHub Action
Status](https://github.com/IntelligentRoboticsLabs/ros2_planning_system/workflows/master/badge.svg)](https://github.com/Docencia-fmrico/patrolling-roscon)

# PATROLLING BEHAVIOR BY ROSCON

## Problema a Resolver

Ejercicio 2 de Planificación y Sistemas Cognitivos 2023

En grupos de 4, haced una aplicación en ROS 2 que haga que un robot patrulle a lo largo de al menos 4 waypoints.

* Debes usar Behavior Trees y Nav2.
* Debe navegar en el mundo de Gazebo que prefieras, que hayas mapeado previamente.
* El robot debe realizar alguna acción cuando llegue a un waypoint (mostrar mensaje, girar sobre sí mismo,...)

El robot debe funcionar en el robot Tiago simulado.

Puntuación (sobre 10):

* +8 correcto funcionamiento en el robot simulado.
* +2 Readme.md bien documentado con videos.
* -3 Warnings o que no pase los tests.


## INTEGRANTES

Integrantes:
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

El árbol de comportamiento está formado por un 'Sequence' que tiene tres acciones.
'GetWaypoints' que obtiene las coordenadas del siguiente waypoint siempre que esté dentro del rango. Si ya ha ido a todos los waypoints definido, va a la posición final. En ambos casos, devuelve running. Devuelve 'failure' cuando ya ha alcanzado la posición final.
'Move' que recibe como input las coordenadas del siguiente waypoint y las pasa a Nav2. Espera el resultado de la navegación y cuando llega a la posición indicada, devuelve success. Mientras, devuelve running.
Cuando ejecuta 'Patrol', gira durante 15 segundos y mientras devuelve running. Una vez pasados estos segundos, devuelve success y vuelve a empezar el sequence. 

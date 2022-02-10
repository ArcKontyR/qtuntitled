import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15

Item {

    Connections {
        target: main

        onSetMapCoordinate: {
            mapPolyline.addCoordinate(QtPositioning.coordinate(latitude,
                                                               longitude))
        }
        onSetMapCenter: {
            _map.center = QtPositioning.coordinate(latitude, longitude)
        }
        onSetMapZoomLevel: {
            _map.zoomLevel = zoom
        }
        onClearMapCoordinates: {
            mapPolyline.path = []
        }
        onSetMapType: {
            _map.activeMapType = _map.supportedMapTypes[typeValue]
        }
    }

    Map {
        id: _map
        objectName: "map"
        width: parent.width
        height: parent.height
        plugin: Plugin {
            id: mapPlugin
            name: "osm"

            //            PluginParameter {
            //                name: "osm.mapping.highdpi_tiles"
            //                value: true
            //            }
            PluginParameter {
                name: "osm.mapping.offline.directory"
                value: "file://C:/Users/USER/Documents/Verhoturov/offline_tiles/"
            }
        }
        center: QtPositioning.coordinate(56.394, 61.9334)
        zoomLevel: 12

        MapPolyline {
            id: mapPolyline
            objectName: "mapPolyline"
            path: []
        }

        onCenterChanged: {
            main.setMapInfo(_map.center.latitude, _map.center.longitude,
                            _map.zoomLevel)
        }
        onZoomLevelChanged: {
            main.setMapInfo(_map.center.latitude, _map.center.longitude,
                            _map.zoomLevel)
        }
    }


    /*🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
    🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷*/
}

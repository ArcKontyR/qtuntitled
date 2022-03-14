import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 5.15
import QtPositioning 5.15

Item {
    Connections {
        target: main
        property MapCircle circle
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
            console.log(_map.mapItems.length)
            for (var i = _map.mapItems.length - 1; i >= 0; i--) {
                if (_map.mapItems[i].objectName === "mapCircle") {
                    _map.mapItems[i].destroy()
                    _map.removeMapItem(_map.mapItems[i])
                }
            }
        }
        onSetMapType: {
            _map.activeMapType = _map.supportedMapTypes[typeValue]
        }
        onSetMapOfflineDirectory: {
            osmOfflineDir.value = directory + "osm"
            osmOfflineCacheDir.value = directory + "osm"
            esriCacheDir.value = directory + "esri"
        }
        onSetMapHighDPI: {
            osmHighDPI.value = value
        }
        onSetMapPlugin: {
            if (plugin === "esri") {
                _map.plugin = esriMapPlugin
            } else {
                _map.plugin = osmMapPlugin
            }
        }
        onSetMapLineColor: {
            mapPolyline.line.color = color
        }
        onSetMapLineWidth: {
            mapPolyline.line.width = width
        }
        onSetMapCircleCoordinate: {
            circle = Qt.createQmlObject('import QtLocation 5.15; MapCircle {}',
                                        _map)
            circle.objectName = "mapCircle"
            circle.center = QtPositioning.coordinate(latitude, longitude)
            _map.addMapItem(circle)
        }
        onSetMapCircleRadius: {
            for (var i = _map.mapItems.length - 1; i >= 0; i--) {
                if (_map.mapItems[i].objectName === "mapCircle") {
                    _map.mapItems[i].radius = radius
                }
            }
        }
        onSetMapCircleBorderColor: {
            for (var i = _map.mapItems.length - 1; i >= 0; i--) {
                if (_map.mapItems[i].objectName === "mapCircle") {
                    _map.mapItems[i].border.color = bordercolor
                }
            }
        }
        onSetMapCircleBorderWidth: {
            for (var i = _map.mapItems.length - 1; i >= 0; i--) {
                if (_map.mapItems[i].objectName === "mapCircle") {
                    _map.mapItems[i].border.width = borderwidth
                }
            }
        }
        onSetMapCircleColor: {
            for (var i = _map.mapItems.length - 1; i >= 0; i--) {
                if (_map.mapItems[i].objectName === "mapCircle") {
                    _map.mapItems[i].color = color
                }
            }
        }
    }

    Plugin {
        id: osmMapPlugin
        name: "osm"

        PluginParameter {
            id: osmHighDPI
            name: "osm.mapping.highdpi_tiles"
        }
        PluginParameter {
            id: osmOfflineCacheDir
            name: "osm.mapping.cache.directory"
        }
        PluginParameter {
            id: osmOfflineDir
            name: "osm.mapping.offline.directory"
        }
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: "true"
        }
        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: "http://maps-redirect.qt.io/osm/6.2/"
        }
    }

    Plugin {
        id: esriMapPlugin
        name: "esri"
        locales: "ru"

        PluginParameter {
            id: esriCacheDir
            name: "esri.mapping.cache.directory"
        }
    }

    Map {
        id: _map
        objectName: "map"
        width: parent.width
        height: parent.height
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


    /*🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷🕷
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

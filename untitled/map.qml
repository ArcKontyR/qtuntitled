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
        onSetMapOfflineDirectory: {
            osmOfflineDir.value = directory + "osm"
            osmOfflineCacheDir.value = directory + "osm"
            esriCacheDir.value = directory + "esri"
        }
        onSetMapHighDPI: {
            osmHighDPI.value = value
            //console.log(osmHighDPI.value, value)
        }
        onSetMapPlugin: {
            if (plugin == "esri") {
                _map.plugin = esriMapPlugin
            } else {
                _map.plugin = osmMapPlugin
            }
            console.log(plugin, _map.plugin)
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
            id: esriMaxZoomLevel
            name: "esri.mapping.maximumZoomLevel"
            value: 18
        }
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
        onSupportedMapTypesChanged: {
            console.log("Supported MapType:")
            for (var i = 0; i < _map.supportedMapTypes.length; i++) {
                console.log(i, supportedMapTypes[i].name)
            }
        }
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

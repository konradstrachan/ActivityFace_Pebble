module.exports = [
    { 
        "type": "heading", 
        "defaultValue": "ActivityFace" 
    }, 
    { 
        "type": "text", 
        "defaultValue": "The watchface has a friendly emoji next to the date that will provide feedback based on how active you are relative to your historical activity for the previous week. This small little icon provides a little personality to your wrist." 
    },
    {
        "type": "toggle",
        "messageKey": "CONFIG_SHOWEMOJI",
        "label": "Show emoji motivator",
        "defaultValue" : true
    },
    {
        "type": "radiogroup",
        "messageKey": "CONFIG_DATEFORMAT",
        "label": "The date format is configurable depending on your preferences.",
        "options": 
        [
            { 
                "label": "DD/MM", 
                "value": "0",
            },
            { 
                "label": "MM/DD", 
                "value": "1",
            }
        ],
        "defaultValue" : "0"
    },
    {
        "type": "submit",
        "defaultValue": "Save"
    }
];
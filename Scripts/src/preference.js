import fs from 'fs-plus'
import _ from 'lodash'
import path from 'path'
let preferenceFile;
let pref;
function loadPreference() {
    preferenceFile = path.join(fs.getHomeDirectory(), '.azure_iot_pref.json');
    if (fs.isFileSync(preferenceFile)) {
        try {
            pref = JSON.parse(fs.readFileSync(preferenceFile, 'utf-8'));
        } catch (err) {
            pref = {};
        }
    } else {
        pref = {};
    }
}
export function getValue(key) {
    if (!_.isString(key)) {
        throw new Error("Preference key must be string.");
    }
    return pref[key];
}
export function setValue(key, value) {
    if (!_.isString(key)) {
        throw new Error("Preference key must be string.");
    }
    if (_.isNull(value) || _.isString(value) || _.isNumber(value) ||  _.isUndefined(value)) {
        pref[key] = value;
    } else {
        throw new Error("Preference value must be simple type.");
    }

}
export function savePreference() {
    preferenceFile = path.join(fs.getHomeDirectory(), '.azure_iot_pref.json');
    fs.writeFileSync(preferenceFile, JSON.stringify(pref, null, 4));
}

loadPreference();
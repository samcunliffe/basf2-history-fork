/*

Support functions for using Ractive in Validation website

*/

function default_values(keypath) {
    // Default configuration values. This is what we use when we can't find
    // a configuration value in localStorage and sessionStorage.
    // Null is returned, if no default value was set. In this case, also
    // a warning is issued to the log.
    let defaults = {
        "show_overview": true,
        "show_expert_plots": false
    };
    if (defaults.hasOwnProperty(keypath)) {
        return defaults[keypath];
    } else {
        console.warn(
            `No default value found for keypath '${keypath}'. Returning null.`
        );
        return null;
    }
}

function convert_string_values(val) {
    // string representations of true and false need to be converted back
    if (val === "false") {
        return false;
    } else if (val === "true") {
        return true;
    } else if (val === "null") {
        return null;
    } else if (val === "undefined") {
        return null;
    }

    return val;
}

function ractive_value_recover_session(ractive, keypath) {
    // Finds value corresponding to $keypath in the sessionStorage.
    // If the key does not exist in storage, the default value from the
    // default_values function is taken.
    // The value is then set in ractive using ractive.set
    let key = get_storage_id(keypath);
    let val = convert_string_values(sessionStorage.getItem(key));
    if (val === null) {
        val = default_values(keypath);
        console.debug(
            `Did not find key '${key}' in session storage, so it was set ` +
            `to default value '${val}'.`
        );
    }
    ractive.set(keypath, val);
}

function ractive_value_recover_local(ractive, keypath) {
    // Finds value corresponding to $keypath either in the localStorage.
    // If the key does not exist in storage, the default value from the
    // default_values function is taken.
    // The value is then set in ractive using ractive.set
    let key = get_storage_id(keypath);
    let val = convert_string_values(localStorage.getItem(key));
    if (val === null) {
        val = default_values(keypath);
        console.debug(
            `Did not find key '${key}' in local storage, so it was set ` +
            `to default value '${val}'.`
        );
    }
    ractive.set(keypath, val);
}


function ractive_value_preserve_session(ractive, keypath) {
    // Saves value associated with $keypath in ractive to sessionStorage.
    let val = ractive.get(keypath);
    let key = get_storage_id(keypath);
    sessionStorage.setItem(key, val);
    console.debug(`Storing key '${key}' with value '${val}' to session storage`);
}

function ractive_value_preserve_local(ractive, keypath) {
    let val = ractive.get(keypath);
    let key = get_storage_id(keypath);
    localStorage.setItem(key, val);
    console.debug(`Storing key '${key}' with value '${val}' to local storage`);

}

function setupRactive(templateName,
                      element,
                      data,
                      onRactiveCreated,
                      onRactiveTemplateComplete,
                      onRactiveTeardown,
                      onRactiveRender,
                      onRactiveChange) {

    console.log(`Setting up Ractive with template '${templateName}'.`);
    $.get(`templates/${templateName}.html`).then(
        function (html_template) {
            // IMPORTANT: DO NOT CHANGE THIS 'var' TO 'let'!
            var ractive = new Ractive({
                // The `el` option can be a node, an ID, or a CSS selector.
                el: element,
                // We could pass in a string, but for the sake of convenience
                // we're passing the ID of the <script> tag above.
                template: html_template,
                // Here, we're passing in some initial data
                data: data,
                oncomplete: function () {
                    if (typeof onRactiveTemplateComplete != 'undefined') {
                        onRactiveTemplateComplete(ractive);
                    }
                },
                onteardown: function () {
                    if (typeof onRactiveTeardown != 'undefined') {
                        onRactiveTeardown(ractive);
                    }
                },
                onrender: function () {
                    if (typeof onRactiveRender != 'undefined') {
                        onRactiveRender(ractive);
                    }
                },
                onchange: function () {
                    if (typeof onRactiveChange != 'undefined') {
                        onRactiveChange(ractive);
                    }
                }

            });

            if (onRactiveCreated != null) {
                onRactiveCreated(ractive);
            }

            // Reload MathJax, because we might have new LaTeX code in Ractive
            // elements. It might be a slight overkill to do this every time,
            // but it's not costly performance wise anyhow, so let's just do it.
            // In fact, because MathJax replaces the DOMs with pictures, it will
            // not regenerate things twice.
            let t0 = performance.now();
            MathJax.Hub.Queue(["Typeset", MathJax.Hub]);
            let t1 = performance.now();
            console.log(`Re-Typeset all using MathJax. This took ${t1 - t0}ms.`);

            console.log(`Ractive setup with template '${templateName}' complete.`);
        }).fail(
        function () {
            alert(`Cannot load ractive template '${templateName}' from webserver.`);
        });
}



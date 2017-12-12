import $ from 'jquery';

class ConfigManager {
	constructor() {
		this.config = false;
	}
	
  fetchConfig() {
    return $.ajax({
      method: "GET",
      url: "config.json"
    })
		.then((result) => {this.config = result});
  }
	
	getConfigValue(key) {
		if (this.config) {
			return this.config[key];
		} else {
			return false;
		}
	}
}

let config = new ConfigManager();

export default config;

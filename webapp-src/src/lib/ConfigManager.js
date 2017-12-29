import $ from 'jquery';
import Cookies from 'js-cookie';

class ConfigManager {
	constructor() {
		this.localStorageKey = "taliesinConfig";
		this.config = false;
	}
	
	fetchConfig() {
		return $.ajax({
			method: "GET",
			url: "config.json"
		})
		.then((result) => {
			this.config = result;
		});
	}
	
	getConfigValue(key) {
		if (this.config) {
			return this.config[key];
		} else {
			return false;
		}
	}
	
	getLocalConfigValue(key) {
		var storage = false;
		if (this.config.storageType === "local") {
			storage = JSON.parse(localStorage.getItem(this.localStorageKey));
		} else if (this.config.storageType === "cookie") {
			storage = JSON.parse(Cookies.get(this.localStorageKey));
		}
		
		if (storage && storage[key]) {
			return storage[key];
		} else {
			return false;
		}
	}
	
	getLocalConfig() {
		var storage = false;
		if (this.config.storageType === "local") {
			storage = JSON.parse(localStorage.getItem(this.localStorageKey));
		} else if (this.config.storageType === "cookie") {
			storage = JSON.parse(Cookies.get(this.localStorageKey));
		}
		
		return storage;
	}
	
	setLocalConfigValue(key, value) {
		var storage = false;
		if (this.config.storageType === "local") {
			storage = JSON.parse(localStorage.getItem(this.localStorageKey));
		} else if (this.config.storageType === "cookie") {
			storage = JSON.parse(Cookies.get(this.localStorageKey));
		}
		
		if (!storage) {
			storage = {};
		}
		
		storage[key] = value;
		if (this.config.storageType === "local") {
			return localStorage.setItem(this.localStorageKey, JSON.stringify(storage));
		} else if (this.config.storageType === "cookie") {
			return Cookies.set(this.localStorageKey, JSON.stringify(storage));
		} else {
			return false;
		}
	}
}

let config = new ConfigManager();

export default config;

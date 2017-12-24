import $ from 'jquery';
import StateStore from './StateStore';

class APIManager {
	constructor(parameters) {
		if (parameters) {
			this.taliesinApiUrl = parameters.taliesinApiUrl || "";
			this.angharadApiUrl = parameters.angharadApiUrl || "";
		}
	}
	
	APIRequest (method, url, data) {
		return $.ajax({
			method: method,
			url: url,
			data: JSON.stringify(data),
			contentType: data?"application/json; charset=utf-8":null,
			headers: {"Authorization": "Bearer " + StateStore.getState().token}
		});
	}
	
	angharadApiRequest(method, url, data) {
		if (this.angharadApiUrl) {
			return this.APIRequest(method, this.angharadApiUrl + url, data);
		} else {
			return Promise.reject("no angharad API");
		}
	}
	
	taliesinApiRequest(method, url, data) {
		if (this.taliesinApiUrl) {
			return this.APIRequest(method, this.taliesinApiUrl + url, data);
		} else {
			return Promise.reject("no taliesin API");
		}
	}
}

export default APIManager;

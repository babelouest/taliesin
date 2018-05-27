import $ from 'jquery';
import StateStore from './StateStore';

class APIManager {
	constructor(parameters) {
		this.counter = 0;
		
		if (parameters) {
			this.taliesinApiUrl = parameters.taliesinApiUrl || "";
			this.angharadApiUrl = parameters.angharadApiUrl || "";
			this.oauth2 = !!parameters.oauth2;
		}
	}
	
	APIRequest (method, url, data) {
		if (this.counter <= 100) {
			this.counter++;
			var curDate = new Date();
			if (!this.oauth2 || StateStore.getState().token_expiration*1000 > curDate.getTime()) {
				return this.APIRequestExecute(method, url, data)
				.always(() => {
					this.counter--;
				});
			} else {
				if (StateStore.getState().oauth2Connector && StateStore.getState().oauth2Connector.runRefreshToken) {
					return StateStore.getState().oauth2Connector.runRefreshToken()
					.then(() => {
						return this.APIRequestExecute(method, url, data);
					})
					.always(() => {
						this.counter--;
					});
				} else {
					this.counter--;
					return $.Deferred().reject("error oauth2Connector").promise();
				}
			}
		} else {
			return $.Deferred().reject("error too busy").promise();
		}
	}
	
	APIRequestExecute(method, url, data) {
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
			return $.Deferred().reject("no angharad API").promise();
		}
	}
	
	taliesinApiRequest(method, url, data) {
		if (this.taliesinApiUrl) {
			return this.APIRequest(method, this.taliesinApiUrl + url, data);
		} else {
			return $.Deferred().reject("no taliesin API").promise();
		}
	}
}

export default APIManager;

class Routage {

	constructor() {
    this.state = 0;
    this.changeCallback = [];

    window.onhashchange = () => {
      this.changeCallback.forEach((cb) => {
        cb(this.getCurrentRoute());
      });
    };
	}

  addChangeRouteCallback(cb) {
    this.changeCallback.push(cb);
  }

  getCurrentRoute() {
    var route = window.location.hash;
    if (route) {
      return route.substring(1);
    } else {
      return false;
    }
  }

  addRoute(route) {
    history.pushState({id: "state-"+(this.state++)}, "", "#" + route);
  }
}

let routage = new Routage();

export default routage;

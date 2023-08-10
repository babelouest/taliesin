import React, { Component } from 'react';
import NotificationSystem from 'react-notification-system';
import { I18n } from 'react-i18next';
import i18n from './lib/i18n';

import TopMenu from './TopMenu/TopMenu';
import MainScreen from './MainScreen/MainScreen';
import FullScreen from './MainScreen/FullScreen';
import Footer from './Footer/Footer';
import StateStore from './lib/StateStore';
import routage from './lib/Routage';

class App extends Component {
  constructor(props) {
    super(props);
    this.gotoRoute(routage.getCurrentRoute());
    
    this.gotoRoute = this.gotoRoute.bind(this);
  }

  gotoRoute(route) {
    if (route) {
      if (route === "playerInternal") {
        StateStore.dispatch({type: "setCurrentPlayer", currentPlayer: {type: "internal", name: i18n.t("player.internal")}});
      } else if (route === "playerExternal") {
        StateStore.dispatch({type: "setCurrentPlayer", currentPlayer: {type: "external", name: i18n.t("player.external")}});
      } else if (route.startsWith("playerCarleon/")) {
        StateStore.dispatch({type: "setCurrentPlayer", currentPlayer: {type: "carleon", name: route.split("/")[1]}});
      }
    }
  }

	componentDidMount() {
		StateStore.dispatch({ type: 'newNotificationManager', notificationManager: this.refs.notificationSystem });
	}
	
	render() {
		return (
			<div className="container-fluid">
				<NotificationSystem ref="notificationSystem" />
				<I18n ns="translations">
					{
						(t, { i18n }) => (
							<div>
								<FullScreen />
								<TopMenu/>
								<MainScreen/>
								<Footer/>
							</div>
						)
					}
				</I18n>
			</div>
		);
	}
}

export default App;

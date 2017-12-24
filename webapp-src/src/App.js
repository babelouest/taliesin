import React, { Component } from 'react';
import NotificationSystem from 'react-notification-system';
import { I18n } from 'react-i18next';

import TopMenu from './TopMenu/TopMenu';
import MainScreen from './MainScreen/MainScreen';
import FullScreen from './MainScreen/FullScreen';
import Footer from './Footer/Footer';
import StateStore from './lib/StateStore';

class App extends Component {
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

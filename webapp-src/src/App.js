import React, { Component } from 'react';
import NotificationSystem from 'react-notification-system';
import TopMenu from './TopMenu/TopMenu';
import MainScreen from './MainScreen/MainScreen';
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
				<TopMenu/>
				<MainScreen/>
				<Footer/>
			</div>
		);
	}
}

export default App;

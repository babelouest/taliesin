import React, { Component } from 'react';
import { Row, Col, Button } from 'react-bootstrap';
import { WithContext as ReactTags } from 'react-tag-input';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class ManageConfig extends Component {	
	constructor(props) {
		super(props);
		
		this.state = {
			audioExtensions: [],
			videoExtensions: [],
			subtitleExtensions: [],
			imageExtensions: [],
			coverFilePattern: []
		};
		
		this.getConfigValues();
		
		this.getConfigValues = this.getConfigValues.bind(this);
		
		this.handleDeleteAudioExtensions = this.handleDeleteAudioExtensions.bind(this);
		this.handleAdditionAudioExtensions = this.handleAdditionAudioExtensions.bind(this);
		this.handleDragAudioExtensions = this.handleDragAudioExtensions.bind(this);
		this.saveAudioExtensions = this.saveAudioExtensions.bind(this);
		
		this.handleDeleteVideoExtensions = this.handleDeleteVideoExtensions.bind(this);
		this.handleAdditionVideoExtensions = this.handleAdditionVideoExtensions.bind(this);
		this.handleDragVideoExtensions = this.handleDragVideoExtensions.bind(this);
		this.saveVideoExtensions = this.saveVideoExtensions.bind(this);
		
		this.handleDeleteSubtitleExtensions = this.handleDeleteSubtitleExtensions.bind(this);
		this.handleAdditionSubtitleExtensions = this.handleAdditionSubtitleExtensions.bind(this);
		this.handleDragSubtitleExtensions = this.handleDragSubtitleExtensions.bind(this);
		this.saveSubtitleExtensions = this.saveSubtitleExtensions.bind(this);
		
		this.handleDeleteImageExtensions = this.handleDeleteImageExtensions.bind(this);
		this.handleAdditionImageExtensions = this.handleAdditionImageExtensions.bind(this);
		this.handleDragImageExtensions = this.handleDragImageExtensions.bind(this);
		this.saveImageExtensions = this.saveImageExtensions.bind(this);
		
		this.handleDeleteCoverFilePattern = this.handleDeleteCoverFilePattern.bind(this);
		this.handleAdditionCoverFilePattern = this.handleAdditionCoverFilePattern.bind(this);
		this.handleDragCoverFilePattern = this.handleDragCoverFilePattern.bind(this);
		this.saveCoverFilePattern = this.saveCoverFilePattern.bind(this);
	}
	
	getConfigValues() {
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/config/audio_file_extension")
		.then((result) => {
			var tab = [];
			result.forEach((res, index) => {
				tab.push({id: index, text: res});
			});
			this.setState({audioExtensions: tab});
		});
		
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/config/video_file_extension")
		.then((result) => {
			var tab = [];
			result.forEach((res, index) => {
				tab.push({id: index, text: res});
			});
			this.setState({videoExtensions: tab});
		});
		
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/config/subtitle_file_extension")
		.then((result) => {
			var tab = [];
			result.forEach((res, index) => {
				tab.push({id: index, text: res});
			});
			this.setState({subtitleExtensions: tab});
		});
		
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/config/image_file_extension")
		.then((result) => {
			var tab = [];
			result.forEach((res, index) => {
				tab.push({id: index, text: res});
			});
			this.setState({imageExtensions: tab});
		});
		
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/config/cover_file_pattern")
		.then((result) => {
			var tab = [];
			result.forEach((res, index) => {
				tab.push({id: index, text: res});
			});
			this.setState({coverFilePattern: tab});
		});
	}
	
	handleDeleteAudioExtensions(i) {
		let tags = this.state.audioExtensions;
		tags.splice(i, 1);
		this.setState({audioExtensions: tags});
	}

	handleAdditionAudioExtensions(tag) {
		let tags = this.state.audioExtensions;
		tags.push({
			id: tags.length + 1,
			text: tag
		});
		this.setState({audioExtensions: tags});
	}

	handleDragAudioExtensions(tag, currPos, newPos) {
		let tags = this.state.audioExtensions;

		// mutate array 
		tags.splice(currPos, 1);
		tags.splice(newPos, 0, tag);

		// re-render 
		this.setState({ audioExtensions: tags });
	}
	
	saveAudioExtensions() {
		var data = [];
		this.state.audioExtensions.forEach((val) => {
			data.push(val.text);
		});
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/config/audio_file_extension", data)
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_success"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_error"),
				level: 'error'
			});
		});
	}
	
	handleDeleteVideoExtensions(i) {
		let tags = this.state.videoExtensions;
		tags.splice(i, 1);
		this.setState({videoExtensions: tags});
	}

	handleAdditionVideoExtensions(tag) {
		let tags = this.state.videoExtensions;
		tags.push({
			id: tags.length + 1,
			text: tag
		});
		this.setState({videoExtensions: tags});
	}

	handleDragVideoExtensions(tag, currPos, newPos) {
		let tags = this.state.videoExtensions;

		// mutate array 
		tags.splice(currPos, 1);
		tags.splice(newPos, 0, tag);

		// re-render 
		this.setState({ videoExtensions: tags });
	}
	
	saveVideoExtensions() {
		var data = [];
		this.state.videoExtensions.forEach((val) => {
			data.push(val.text);
		});
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/config/video_file_extension", data)
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_success"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_error"),
				level: 'error'
			});
		});
	}
	
	handleDeleteSubtitleExtensions(i) {
		let tags = this.state.subtitleExtensions;
		tags.splice(i, 1);
		this.setState({subtitleExtensions: tags});
	}

	handleAdditionSubtitleExtensions(tag) {
		let tags = this.state.subtitleExtensions;
		tags.push({
			id: tags.length + 1,
			text: tag
		});
		this.setState({subtitleExtensions: tags});
	}

	handleDragSubtitleExtensions(tag, currPos, newPos) {
		let tags = this.state.subtitleExtensions;

		// mutate array 
		tags.splice(currPos, 1);
		tags.splice(newPos, 0, tag);

		// re-render 
		this.setState({ subtitleExtensions: tags });
	}
	
	saveSubtitleExtensions() {
		var data = [];
		this.state.subtitleExtensions.forEach((val) => {
			data.push(val.text);
		});
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/config/subtitle_file_extension", data)
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_success"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_error"),
				level: 'error'
			});
		});
	}
	
	handleDeleteImageExtensions(i) {
		let tags = this.state.imageExtensions;
		tags.splice(i, 1);
		this.setState({imageExtensions: tags});
	}

	handleAdditionImageExtensions(tag) {
		let tags = this.state.imageExtensions;
		tags.push({
			id: tags.length + 1,
			text: tag
		});
		this.setState({imageExtensions: tags});
	}

	handleDragImageExtensions(tag, currPos, newPos) {
		let tags = this.state.imageExtensions;

		// mutate array 
		tags.splice(currPos, 1);
		tags.splice(newPos, 0, tag);

		// re-render 
		this.setState({ imageExtensions: tags });
	}
	
	saveImageExtensions() {
		var data = [];
		this.state.imageExtensions.forEach((val) => {
			data.push(val.text);
		});
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/config/image_file_extension", data)
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_success"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_error"),
				level: 'error'
			});
		});
	}
	
	handleDeleteCoverFilePattern(i) {
		let tags = this.state.coverFilePattern;
		tags.splice(i, 1);
		this.setState({coverFilePattern: tags});
	}

	handleAdditionCoverFilePattern(tag) {
		let tags = this.state.coverFilePattern;
		tags.push({
			id: tags.length + 1,
			text: tag
		});
		this.setState({coverFilePattern: tags});
	}

	handleDragCoverFilePattern(tag, currPos, newPos) {
		let tags = this.state.coverFilePattern;

		// mutate array 
		tags.splice(currPos, 1);
		tags.splice(newPos, 0, tag);

		// re-render 
		this.setState({ coverFilePattern: tags });
	}
	
	saveCoverFilePattern() {
		var data = [];
		this.state.coverFilePattern.forEach((val) => {
			data.push(val.text);
		});
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/config/cover_file_pattern", data)
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_success"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("config.message_save_error"),
				level: 'error'
			});
		});
	}
	
	render() {
		return (
			<div>
				<Row>
					<Col md={12}>
						<h3>
							{i18n.t("config.audioExtensions")}
						</h3>
					</Col>
				</Row>
				<Row>
					<Col md={11} sm={11} xs={11}>
						<ReactTags tags={this.state.audioExtensions}
								handleDelete={this.handleDeleteAudioExtensions}
								handleAddition={this.handleAdditionAudioExtensions}
								handleDrag={this.handleDragAudioExtensions} />
					</Col>
					<Col md={1} sm={1} xs={1}>
						<Button bsStyle="primary" onClick={this.saveAudioExtensions}>{i18n.t("common.save")}</Button>
					</Col>
				</Row>
				<Row>
					<hr />
				</Row>
				<Row>
					<Col md={12}>
						<h3>
							{i18n.t("config.videoExtensions")}
						</h3>
					</Col>
				</Row>
				<Row>
					<Col md={11} sm={11} xs={11}>
						<ReactTags tags={this.state.videoExtensions}
								handleDelete={this.handleDeleteVideoExtensions}
								handleAddition={this.handleAdditionVideoExtensions}
								handleDrag={this.handleDragVideoExtensions} />
					</Col>
					<Col md={1} sm={1} xs={1}>
						<Button bsStyle="primary" onClick={this.saveVideoExtensions}>{i18n.t("common.save")}</Button>
					</Col>
				</Row>
				<Row>
					<hr />
				</Row>
				<Row>
					<Col md={12}>
						<h3>
							{i18n.t("config.subtitleExtensions")}
						</h3>
					</Col>
				</Row>
				<Row>
					<Col md={11} sm={11} xs={11}>
						<ReactTags tags={this.state.subtitleExtensions}
								handleDelete={this.handleDeleteSubtitleExtensions}
								handleAddition={this.handleAdditionSubtitleExtensions}
								handleDrag={this.handleDragSubtitleExtensions} />
					</Col>
					<Col md={1} sm={1} xs={1}>
						<Button bsStyle="primary" onClick={this.saveSubtitleExtensions}>{i18n.t("common.save")}</Button>
					</Col>
				</Row>
				<Row>
					<hr />
				</Row>
				<Row>
					<Col md={12}>
						<h3>
							{i18n.t("config.imageExtensions")}
						</h3>
					</Col>
				</Row>
				<Row>
					<Col md={11} sm={11} xs={11}>
						<ReactTags tags={this.state.imageExtensions}
								handleDelete={this.handleDeleteImageExtensions}
								handleAddition={this.handleAdditionImageExtensions}
								handleDrag={this.handleDragImageExtensions} />
					</Col>
					<Col md={1} sm={1} xs={1}>
						<Button bsStyle="primary" onClick={this.saveImageExtensions}>{i18n.t("common.save")}</Button>
					</Col>
				</Row>
				<Row>
					<hr />
				</Row>
				<Row>
					<Col md={12}>
						<h3>
							{i18n.t("config.coverFilePattern")}
						</h3>
					</Col>
				</Row>
				<Row>
					<Col md={11} sm={11} xs={11}>
						<ReactTags tags={this.state.coverFilePattern}
								handleDelete={this.handleDeleteCoverFilePattern}
								handleAddition={this.handleAdditionCoverFilePattern}
								handleDrag={this.handleDragCoverFilePattern} />
					</Col>
					<Col md={1} sm={1} xs={1}>
						<Button bsStyle="primary" onClick={this.saveCoverFilePattern}>{i18n.t("common.save")}</Button>
					</Col>
				</Row>
				<Row>
					<hr />
				</Row>
			</div>
		);
	}
}

export default ManageConfig;

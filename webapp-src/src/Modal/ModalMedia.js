import React, { Component } from 'react';
import { Modal, Row, Col, Image, Button } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';

class ModalMedia extends Component {
  constructor(props) {
    super(props);
		
		this.state = {show: props.show, media: props.media, title: props.title, close: props.onClose, imgBlob: false};
		
		this.onCloseModal = this.onCloseModal.bind(this);
		this.getMediaCover = this.getMediaCover.bind(this);
		this.handleSelectDataSource = this.handleSelectDataSource.bind(this);
		this.handleSelectArtist = this.handleSelectArtist.bind(this);
		this.handleSelectAlbum = this.handleSelectAlbum.bind(this);
		this.handleSelectYear = this.handleSelectYear.bind(this);
		this.handleSelectGenre = this.handleSelectGenre.bind(this);
		this.onPlayNow = this.onPlayNow.bind(this);
		this.runPlayNow = this.runPlayNow.bind(this);
		
		this.getMediaCover();
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({show: nextProps.show, media: nextProps.media, title: nextProps.title, close: nextProps.onClose, imgBlob: false}, () => {
			this.getMediaCover();
		});
	}

  onCloseModal() {
    this.setState({show: false}, () => {
			if (this.state.close) {
				this.state.close();
			}
		});
  }
	
	onPlayNow() {
		var streamList = StateStore.getState().streamList, curStream = streamList.find((stream) => {return stream.display_name.startsWith("{" + (StateStore.getState().profile.currentPlayer||"local") + "}")});
		if (curStream) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(curStream.name) + "/manage", {command: "stop"})
			.then(() => {
				var streamList = StateStore.getState().streamList;
				for (var i in streamList) {
					if (streamList[i].name === curStream.name) {
						streamList.splice(i, 1);
						break;
					}
				}
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				if (StateStore.getState().profile.stream.name === curStream.name) {
					StateStore.dispatch({type: "loadStream", stream: false});
				}
				this.runPlayNow();
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error delete stream',
					level: 'error'
				});
			});
		} else {
			this.runPlayNow();
		}
	}
	
	runPlayNow() {
    StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23") + "?jukebox&recursive&name={" + (StateStore.getState().profile.currentPlayer||"local") + "} - " + (this.state.media.tags.title||this.state.media.name))
    .then((result) => {
			var streamList = StateStore.getState().streamList;
      streamList.push(result);
      StateStore.dispatch({type: "setStreamList", streamList: streamList});
      StateStore.dispatch({type: "loadStreamAndPlay", stream: result, index: 0});
			StateStore.getState().NotificationManager.addNotification({
				message: 'Play new stream',
				level: 'info'
			});
			this.setState({show: false});
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error Play',
				level: 'error'
			});
    });
	}
	
	getMediaCover() {
		this.state.media && StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.media.data_source) + "/browse/path/" + encodeURI(this.state.media.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&base64")
		.then((result) => {
			this.setState({imgBlob: result});
		})
		.fail(() => {
			this.setState({imgBlob: false});
		});
	}
	
	handleSelectDataSource() {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
			StateStore.dispatch({type: "setCurrentPath", path: ""});
		});
	}
	
	handleSelectArtist(value) {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentCategory", category: "artist", categoryValue: value});
		});
	}
	
	handleSelectAlbum(value) {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentCategory", category: "album", categoryValue: value});
		});
	}
	
	handleSelectYear(value) {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentCategory", category: "year", categoryValue: value});
		});
	}
	
	handleSelectGenre(value) {
		this.setState({show: false}, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.media.data_source})});
			StateStore.dispatch({type: "setCurrentCategory", category: "genre", categoryValue: value});
		});
	}
	
  render() {
		var metadata = [];
		var mediaImage = "";
		var separator = "";
		if (this.state.media) {
			if (this.state.media.tags && this.state.media.tags.title) {
				metadata.push(
					<Row key={0}>
						<Col xs={6}>
							<label>Title</label>
						</Col>
						<Col xs={6}>
							<span>{this.state.media.tags.title}</span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags && this.state.media.tags.artist) {
				metadata.push(
					<Row key={1}>
						<Col xs={6}>
							<label>Artist</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" onClick={() => {this.handleSelectArtist(this.state.media.tags.artist)}}>{this.state.media.tags.artist}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags && this.state.media.tags.album) {
				metadata.push(
					<Row key={2}>
						<Col xs={6}>
							<label>Album</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" onClick={() => {this.handleSelectAlbum(this.state.media.tags.album)}}>{this.state.media.tags.album}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags && this.state.media.tags.date) {
				metadata.push(
					<Row key={3}>
						<Col xs={6}>
							<label>Date</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" onClick={() => {this.handleSelectYear(this.state.media.tags.date)}}>{this.state.media.tags.date}</a></span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags && this.state.media.tags.genre) {
				metadata.push(
					<Row key={4}>
						<Col xs={6}>
							<label>Genre</label>
						</Col>
						<Col xs={6}>
							<span><a role="button" onClick={() => {this.handleSelectGenre(this.state.media.tags.genre)}}>{this.state.media.tags.genre}</a></span>
						</Col>
					</Row>);
			}
			if (metadata.length) {
				separator = <Row><hr/></Row>;
			}
			if (this.state.imgBlob) {
				mediaImage = <Image src={"data:image/jpeg;base64," + this.state.imgBlob} className="cover-image-full center-block" responsive />;
			}
			return (
					<Modal show={this.state.show} onHide={this.onCloseModal}>
						<Modal.Header closeButton>
							<Modal.Title>
								<Button onClick={this.onPlayNow} className="btn" title="Play now">
									<FontAwesome name={"play"} />
								</Button>&nbsp;
								{this.state.title}
							</Modal.Title>
						</Modal.Header>
						<Modal.Body>
							{metadata}
							{separator}
							<Row>
								<Col xs={6}>
									<label>Data Source</label>
								</Col>
								<Col xs={6}>
									<span><a role="button" onClick={this.handleSelectDataSource}>{this.state.media.data_source}</a></span>
								</Col>
							</Row>
							<Row>
								<Col xs={6}>
									<label>Name</label>
								</Col>
								<Col xs={6}>
									<span>{this.state.media.name}</span>
								</Col>
							</Row>
							<Row>
								<Col xs={6}>
									<label>Path</label>
								</Col>
								<Col xs={6}>
									<span>{this.state.media.path}</span>
								</Col>
							</Row>
							<Row>
								<hr/>
							</Row>
							<Row>
								<Col xs={12} className="text-center">
									{mediaImage}
								</Col>
							</Row>
							<Row style={{marginTop: "10px"}}>
								<Col xs={12} className="text-center">
									<Button onClick={this.onCloseModal} className="btn btn-success">
										Close
									</Button>
								</Col>
							</Row>
						</Modal.Body>
					</Modal>
			);
		} else {
			return (<div></div>);
		}
	}
}

export default ModalMedia;

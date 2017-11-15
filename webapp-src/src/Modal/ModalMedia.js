import React, { Component } from 'react';
import { Modal, Row, Col, Image } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class ModalMedia extends Component {
  constructor(props) {
    super(props);
		
		this.state = {show: props.show, media: props.media, title: props.title, imgBlob: false};
		
		this.onCloseModal = this.onCloseModal.bind(this);
		this.getMediaCover = this.getMediaCover.bind(this);
		
		this.getMediaCover();
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({show: nextProps.show, media: nextProps.media, title: nextProps.title, imgBlob: false}, () => {
			this.getMediaCover();
		});
	}

  onCloseModal() {
    this.setState({show: false});
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
	
  render() {
		var metadata = [];
		var mediaImage = "";
		var separator = "";
		if (this.state.media) {
			if (this.state.media.tags.title) {
				metadata.push(
					<Row key={metadata.length}>
						<Col xs={6}>
							<label>Title</label>
						</Col>
						<Col xs={6}>
							<span>{this.state.media.tags.title}</span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.artist) {
				metadata.push(
					<Row key={metadata.length}>
						<Col xs={6}>
							<label>Artist</label>
						</Col>
						<Col xs={6}>
							<span>{this.state.media.tags.artist}</span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.album) {
				metadata.push(
					<Row key={metadata.length}>
						<Col xs={6}>
							<label>Album</label>
						</Col>
						<Col xs={6}>
							<span>{this.state.media.tags.album}</span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.date) {
				metadata.push(
					<Row key={metadata.length}>
						<Col xs={6}>
							<label>Date</label>
						</Col>
						<Col xs={6}>
							<span>{this.state.media.tags.date}</span>
						</Col>
					</Row>);
			}
			if (this.state.media.tags.genre) {
				metadata.push(
					<Row key={metadata.length}>
						<Col xs={6}>
							<label>Genre</label>
						</Col>
						<Col xs={6}>
							<span>{this.state.media.tags.genre}</span>
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
							<Modal.Title>{this.state.title}</Modal.Title>
						</Modal.Header>
						<Modal.Body>
							{metadata}
							{separator}
							<Row>
								<Col xs={6}>
									<label>Data Source</label>
								</Col>
								<Col xs={6}>
									<span>{this.state.media.data_source}</span>
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
						</Modal.Body>
					</Modal>
			);
		} else {
			return (<div></div>);
		}
	}
}

export default ModalMedia;

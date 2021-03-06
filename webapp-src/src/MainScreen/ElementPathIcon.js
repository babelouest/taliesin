import React, { Component } from 'react';
import FontAwesome from 'react-fontawesome';
import { Image, Col } from 'react-bootstrap';
import VisibilitySensor from 'react-visibility-sensor';

import StateStore from '../lib/StateStore';
import ElementButtons from './ElementButtons';
import ModalMedia from '../Modal/ModalMedia';

class ElementPathIcon extends Component {
	constructor(props) {
		super(props);
		this.state = {element: props.element, dataSource: props.dataSource, path: props.path, thumb: false, thumbLoaded: false, visible: false, hideRefresh: props.hideRefresh};
		
		this.handleChangePath = this.handleChangePath.bind(this);
		this.handleOpenFile = this.handleOpenFile.bind(this);
		this.onChangeVisibility = this.onChangeVisibility.bind(this);
		this.getThumbnail = this.getThumbnail.bind(this);
		this.handleCloseModal = this.handleCloseModal.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({element: nextProps.element, dataSource: nextProps.dataSource, path: nextProps.path, hideRefresh: nextProps.hideRefresh});
	}
	
	componentDidMount() {
		this._ismounted = true;
		if (this.state.visible && !this.state.thumbLoaded) {
			this.getThumbnail();	
		}
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
	handleChangePath(subPath) {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: StateStore.getState().dataSourceList.find((ds) => {return ds.name === this.state.dataSource})});
		StateStore.dispatch({type: "setCurrentPath", path: (this.state.path?(this.state.path + "/"):"") + subPath });
	}
	
	handleOpenFile(name) {
		var modalMedia = this.state.element;
		modalMedia.data_source = this.state.dataSource;
		this.setState({show: true, modalMedia: modalMedia, modalTitle: this.state.element.name});
	}
	
	handleCloseModal() {
		this.setState({show: false});
	}
	
	onChangeVisibility(isVisible) {
		this.setState({visible: isVisible}, () => {
			if (isVisible && !this.state.thumbLoaded) {
				this.getThumbnail();	
			}
		});
	}
	
	getThumbnail() {
		if (this._ismounted) {
			if (!this.state.thumb && (this.state.element.type === "audio" || this.state.element.type === "folder" || this.state.element.type === "image")) {
				StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "/" + encodeURI(this.state.element.name).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
				.then((result) => {
					this.setState({thumb: result, thumbLoaded: true});
				})
				.fail(() => {
					this.setState({thumb: false, thumbLoaded: true});
				});
			} else {
				this.setState({thumb: false, thumbLoaded: true});
			}
		}
	}
	
	render() {
		var icon = "";
		if (!this.state.thumbLoaded) {
			icon = 
				<div className="text-center">
					<a role="button" onClick={() => this.handleChangePath(this.state.element.name)} title={this.state.element.name}>
						<FontAwesome name="spinner" spin />
						<div className="hideOverflow">
							<span>{this.state.element.name}</span>
						</div>
					</a>
				</div>
		} else {
			if (!this.state.thumb) {
				if (this.state.element.type === "folder") {
					icon =
						<a role="button" onClick={() => this.handleChangePath(this.state.element.name)} title={this.state.element.name}>
							<Image src="images/folder-128.png" alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else if (this.state.element.type === "audio") {
					icon =
						<a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
							<Image src="images/audio-128.png" alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else if (this.state.element.type === "artist") {
					icon =
						<a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
							<Image src="images/artist-128.png" alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else if (this.state.element.type === "album") {
					icon =
						<a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
							<Image src="images/album-128.png" alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else if (this.state.element.type === "year") {
					icon =
						<a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
							<Image src="images/year-128.png" alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else if (this.state.element.type === "genre") {
					icon =
						<a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
							<Image src="images/genre-128.png" alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else {
					icon =
						<a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
							<Image src="images/unknown-128.png" alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				}
			} else {
				if (this.state.element.type === "folder" || this.state.element.type === "artist" || this.state.element.type === "album" || this.state.element.type === "year" || this.state.element.type === "genre") {
					icon =
						<a role="button" onClick={() => this.handleChangePath(this.state.element.name)} title={this.state.element.name}>
							<Image src={"data:image/jpeg;base64,"+this.state.thumb} alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else if (this.state.element.type === "audio") {
					icon =
						<a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
							<Image src={"data:image/jpeg;base64,"+this.state.thumb} alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				} else {
					icon =
						<a role="button" onClick={() => this.handleOpenFile(this.state.element.name)} title={this.state.element.name}>
							<Image src={"data:image/jpeg;base64,"+this.state.thumb} alt={this.state.element.name} className="elementImage" responsive>
							</Image>
							<div className="hideOverflow">
								<span>{this.state.element.name}</span>
							</div>
						</a>
				}
			}
		}
		return (
			<Col md={3} sm={3} xs={6} className="panel panel-default">
				<VisibilitySensor
					scrollCheck
					scrollThrottle={100}
					intervalDelay={8000}
					onChange={this.onChangeVisibility}
					minTopValue={10}
					partialVisibility={true}>
					{icon}
				</VisibilitySensor>
				<div className="text-center">
					<ElementButtons dataSource={this.state.dataSource} path={this.state.path + "/" + this.state.element.name} element={this.state.element} hideRefresh={this.state.hideRefresh}/>
				</div>
				<ModalMedia show={this.state.show} media={this.state.modalMedia} title={this.state.modalTitle} onClose={this.handleCloseModal} />
			</Col>
		);
	}
}

export default ElementPathIcon;

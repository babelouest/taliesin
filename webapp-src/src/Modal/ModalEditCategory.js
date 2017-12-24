import React, { Component } from 'react';
import { Button, Modal, FormControl, Row, Col, Label, Image } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class ModalEditCategory extends Component {
	constructor(props) {
		super(props);
		this._ismounted = false;
		
		this.state = {
			show: props.show,
			dataSource: props.dataSource,
			category: props.category,
			categoryValue: props.categoryValue,
			categoryContent: {},
			categoryContentEdit: {},
			iconPath: "",
			onCloseCb: props.onCloseCb,
			edit: false
		}
		this.loadCategoryContent();
			
		this.loadCategoryContent = this.loadCategoryContent.bind(this);
		this.close = this.close.bind(this);
		this.save = this.save.bind(this);
		this.handleChangeContent = this.handleChangeContent.bind(this);
		this.handleUploadIcon = this.handleUploadIcon.bind(this);
		this.handleEdit = this.handleEdit.bind(this);
	};
	
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			show: nextProps.show,
			dataSource: nextProps.dataSource,
			category: nextProps.category,
			categoryValue: nextProps.categoryValue,
			categoryContent: {},
			categoryContentEdit: {},
			iconPath: "",
			onCloseCb: nextProps.onCloseCb,
			edit: false
		}, () => {
			this.loadCategoryContent();
		});
	}
	
	componentDidMount() {
		this._ismounted = true;
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
	loadCategoryContent() {
		if (this._ismounted && this.state.show) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/info/category/" + encodeURI(this.state.category) + "/" + encodeURI(this.state.categoryValue))
			.then((result) => {
				var categoryContent = this.state.categoryContent;
				categoryContent.content = result.info;
				this.setState({categoryContent: categoryContent}, () => {
					StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/info/category/" + encodeURI(this.state.category) + "/" + encodeURI(this.state.categoryValue) + "?cover&base64")
					.then((cover) => {
						var categoryContent = this.state.categoryContent;
						categoryContent.cover = cover;
						this.setState({categoryContent: categoryContent});
					})
					.fail(() => {
						var categoryContent = this.state.categoryContent;
						categoryContent.cover = false;
						this.setState({categoryContent: categoryContent});
					});
				});
			})
			.fail(() => {
				this.setState({categoryContent: {content: "", cover: false}});
			});
		}
	}

	close(e) {
		if (e) {
			e.preventDefault();
		}
		if (this.state.onCloseCb) {
			this.setState({ show: false }, () => {
				this.state.onCloseCb();
			});
		}
	}
	
	save() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/info/category/" + encodeURI(this.state.category) + "/" + encodeURI(this.state.categoryValue), this.state.categoryContentEdit)
		.then(() => {
			this.setState({categoryContent: this.state.categoryContentEdit, edit: false});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.category_save_ok"),
				level: 'info'
			});
		})
		.fail(() => {
			this.setState({categoryContent: {content: "", cover: false}, edit: false});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.cagegory_save_error"),
				level: 'error'
			});
		});
	}
	
	handleChangeContent(e) {
		var categoryContent = this.state.categoryContent;
		categoryContent.content = e.target.value;
		this.setState({ categoryContent: categoryContent });
	}
	
	handleEdit() {
		if (this.state.edit) {
			this.setState({edit: !this.state.edit});
		} else {
			this.setState({edit: !this.state.edit, categoryContentEdit: this.state.categoryContent});
		}
	}
	
	handleUploadIcon(e) {
		var file = e.target.files[0];
		var fr = new FileReader();
		var self = this;
		if (file.type.startsWith("image") && file.size <= 16*1024*1024) {
			fr.onload = function(ev2) {
				var encoded = btoa(ev2.target.result);
				if (encoded.length <= 16*1024*1024) {
					var categoryContent = self.state.categoryContent;
					categoryContent.cover = encoded;
					self.setState({categoryContent: categoryContent});
				} else {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("modal.file_toot_large"),
						level: 'error'
					});
				}
			};
			fr.readAsBinaryString(file);
		} else {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.file_error"),
				level: 'error'
			});
		}
	}

	render() {
		var cover, coverButton, content;
		if (this.state.categoryContent.cover) {
			cover = 
				<div>
					<Row>
						<Col md={12}>
							<hr/>
						</Col>
					</Row>
					<Row>
						<Col md={4}>
							<Label>{i18n.t("common.image")}</Label>
						</Col>
						<Col md={8}>
							<Image src={"data:image/jpeg;base64," + (this.state.edit?this.state.categoryContentEdit.cover:this.state.categoryContent.cover)} responsive/>
						</Col>
					</Row>
				</div>;
		}
		if (this.state.edit) {
			coverButton = 
				<div>
					<Row>
						<Col md={12}>
							<hr/>
						</Col>
					</Row>
					<Row>
						<Col md={4}>
							<Label>{i18n.t("modal.select_image")}</Label>
						</Col>
						<Col md={8}>
							<FormControl
								type="file"
								placeholder={i18n.t("modal.image_file")}
								onChange={this.handleUploadIcon}
							/>
						</Col>
					</Row>
				</div>;
			content = 
				<FormControl
					componentClass="textarea"
					value={this.state.categoryContentEdit.content}
					onChange={this.handleChangeContent}
				/>;
		} else {
			content = <p>{this.state.categoryContent.content}</p>;
		}
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>
						<Button onClick={this.handleEdit} className="btn" title={i18n.t("common.edit")}>
							<FontAwesome name={"pencil"} />
						</Button>&nbsp;
						{this.state.category}: {this.state.categoryValue}
					</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					<form onSubmit={(e) => {this.close(e)}}>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("modal.content")}</Label>
							</Col>
							<Col md={8}>
								{content}
							</Col>
						</Row>
						{coverButton}
						{cover}
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.save()} disabled={!this.state.edit}>Save</Button>
					<Button bsStyle="primary" onClick={() => this.close()}>Close</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalEditCategory;

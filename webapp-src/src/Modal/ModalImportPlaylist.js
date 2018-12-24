import React, { Component } from 'react';
import { Button, Modal, FormControl, Row, Col, Label } from 'react-bootstrap';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class ModalImportPlaylist extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			show: props.show, 
			importPath: "",
			importData: false,
			onCloseCb: props.onCloseCb
		};
		
		this.close = this.close.bind(this);
		this.handleUploadFile = this.handleUploadFile.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			show: nextProps.show, 
			importPath: "",
			importData: false,
			onCloseCb: nextProps.onCloseCb,
		});
	}

	close(result, e) {
		if (e) {
			e.preventDefault();
		}
		if (result && this.state.onCloseCb) {
			this.setState({ show: false }, () => {
				this.state.onCloseCb(true, this.state.importData);
			});
		} else {
			this.setState({ show: false }, () => {
				this.state.onCloseCb(false, false);
			});
		}
	}
	
	handleUploadFile(e) {
		var file = e.target.files[0];
		var fr = new FileReader();
		var self = this;
		if (file.type.indexOf("json") >= 0) {
			fr.onload = function(ev2) {
				self.setState({importData: ev2.target.result});
			};
			fr.readAsText(file);
		} else {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.import_playlist_file_error"),
				level: 'error'
			});
		}
	}

	render() {
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{i18n.t("modal.title_import_playlist")}</Modal.Title>
				</Modal.Header>

				<Modal.Body>
					<form onSubmit={(e) => {this.close(true, e)}}>
						<Row>
							<Col md={4}>
								<Label>{i18n.t("modal.import_file")}</Label>
							</Col>
							<Col md={8}>
								<FormControl
									type="file"
									placeholder={i18n.t("modal.import_file")}
									onChange={this.handleUploadFile}
								/>
							</Col>
						</Row>
					</form>
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)}>{i18n.t("common.ok")}</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>{i18n.t("common.cancel")}</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalImportPlaylist;

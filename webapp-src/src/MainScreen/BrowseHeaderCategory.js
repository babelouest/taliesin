import React, { Component } from 'react';
import { Col, Row, Image } from 'react-bootstrap';
import BreadCrumbMenuCategory from './BreadCrumbMenuCategory';
import ElementButtons from './ElementButtons';
import StateStore from '../lib/StateStore';
import ModalEditCategory from '../Modal/ModalEditCategory';
import i18n from '../lib/i18n';

class BrowseHeaderCategory extends Component {	
  constructor(props) {
    super(props);
    
    this.state = {
			dataSource: props.dataSource, 
			category: props.category, 
			categoryValue: props.categoryValue, 
			subCategory: props.subCategory, 
			subCategoryValue: props.subCategoryValue, 
			imgThumbBlob: false,
			editCategoryShow: false
		};
    
    this.loadCover = this.loadCover.bind(this);
    this.onOpenCategory = this.onOpenCategory.bind(this);
    this.onCloseCategory = this.onCloseCategory.bind(this);
    this.onEditCategory = this.onEditCategory.bind(this);
    
    this.loadCover();
  }
  
  componentWillReceiveProps(nextProps) {
    this.setState({
			dataSource: nextProps.dataSource, 
			category: nextProps.category, 
			categoryValue: nextProps.categoryValue, 
			subCategory: nextProps.subCategory, 
			subCategoryValue: nextProps.subCategoryValue, 
			imgThumbBlob: false,
			editCategoryShow: false
		}, () => {
      this.loadCover();
    });
  }
	
	onOpenCategory() {
		this.setState({editCategoryShow: true});
	}
  
	onCloseCategory() {
		this.setState({editCategoryShow: false, imgThumbBlob: false}, () => {
			this.loadCover();
		});
	}
	
	onEditCategory() {
		this.setState({imgThumbBlob: false}, () => {
			this.loadCover();
		});
	}
  
  loadCover() {
		if (!this.state.imgThumbBlob) {
			var url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/info/category/";
			if (this.state.subCategoryValue) {
				url += encodeURIComponent(this.state.subCategory) + "/" + encodeURIComponent(this.state.subCategoryValue);
			} else {
				url += encodeURIComponent(this.state.category) + "/" + encodeURIComponent(this.state.categoryValue);
			}
			url += "?cover&thumbnail&base64";
			StateStore.getState().APIManager.taliesinApiRequest("GET", url)
			.then((result) => {
				this.setState({imgThumbBlob: result});
			})
			.fail(() => {
				this.setState({imgThumbBlob: false});
			});
		} else {
			this.setState({imgThumbBlob: false});
		}
  }
  
  render () {
		var element = {type: this.state.subCategory||this.state.category, name: this.state.subCategoryValue||this.state.categoryValue};
    if (this.state.imgThumbBlob) {
      return (
        <Row>
          <Col md={6} sm={6} xs={8}>
						<BreadCrumbMenuCategory dataSource={this.state.dataSource} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} />
          </Col>
          <Col md={3} sm={3} xs={3} className="text-right">
						<a role="button" onClick={this.onOpenCategory} title={i18n.t("browse.view_category_content")}>
							<Image src={"data:image/jpeg;base64," + this.state.imgThumbBlob} thumbnail responsive />
						</a>
          </Col>
          <Col md={3} sm={3} xs={1} className="text-right">
            <div className="text-right">
							<ElementButtons dataSource={this.state.dataSource} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} element={element} />
            </div>
          </Col>
					<ModalEditCategory show={this.state.editCategoryShow} onCloseCb={this.onCloseCategory} dataSource={this.state.dataSource} category={this.state.subCategory||this.state.category} categoryValue={this.state.subCategoryValue||this.state.categoryValue} />
        </Row>
      );
    } else {
      return (
        <Row>
          <Col md={9} sm={9} xs={6}>
            <BreadCrumbMenuCategory dataSource={this.state.dataSource} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} />
          </Col>
          <Col md={3} sm={3} xs={6} className="text-right">
            <div className="text-right">
							<ElementButtons dataSource={this.state.dataSource} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} element={element} onEditCategory={this.onEditCategory} />
            </div>
          </Col>
        </Row>
      );
    }
  }
}

export default BrowseHeaderCategory;

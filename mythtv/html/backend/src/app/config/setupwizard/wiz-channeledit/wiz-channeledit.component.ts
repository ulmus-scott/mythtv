import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { TranslateModule } from '@ngx-translate/core';
import { ButtonModule } from 'primeng/button';
import { ChannelEditorComponent } from '../../settings/channel-editor/channel-editor.component';
import { CardModule } from 'primeng/card';

@Component({
    selector: 'app-wiz-channeledit',
    templateUrl: './wiz-channeledit.component.html',
    styleUrls: ['./wiz-channeledit.component.css'],
    imports: [CardModule, ChannelEditorComponent, ButtonModule, TranslateModule]
})
export class WizChanneleditComponent implements OnInit {

  constructor(public router: Router) { }

  ngOnInit(): void {
  }

}

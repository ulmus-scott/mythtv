import { Component, OnInit, Input } from '@angular/core';
import { Backend } from 'src/app/services/interfaces/backend.interface';
import { BackendStatus } from 'src/app/services/interfaces/status.interface';
import { TranslateModule } from '@ngx-translate/core';


@Component({
    selector: 'app-status-backends',
    templateUrl: './backends.component.html',
    styleUrls: ['./backends.component.css', '../../status.component.css'],
    imports: [TranslateModule]
})
export class BackendsComponent implements OnInit {
  @Input() backends? : Backend[];

  constructor() { }

  ngOnInit(): void {
  }

}
